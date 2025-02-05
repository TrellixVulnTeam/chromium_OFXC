// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/notifications/notifier_state_tracker.h"

#include <stddef.h>

#include <utility>

#include "base/bind.h"
#include "base/values.h"
#include "build/build_config.h"
#include "chrome/browser/permissions/permission_manager.h"
#include "chrome/browser/permissions/permission_result.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/pref_names.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "extensions/features/features.h"
#include "ui/message_center/notifier_settings.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "chrome/common/extensions/api/notifications.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_event_histogram_value.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/extension_util.h"
#include "extensions/browser/info_map.h"
#endif

using message_center::NotifierId;

// static
void NotifierStateTracker::RegisterProfilePrefs(
    user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterListPref(prefs::kMessageCenterDisabledExtensionIds);
  registry->RegisterListPref(prefs::kMessageCenterDisabledSystemComponentIds);
}

NotifierStateTracker::NotifierStateTracker(Profile* profile)
    : profile_(profile)
#if BUILDFLAG(ENABLE_EXTENSIONS)
      ,
      extension_registry_observer_(this)
#endif
{
  OnStringListPrefChanged(
      prefs::kMessageCenterDisabledExtensionIds, &disabled_extension_ids_);
  OnStringListPrefChanged(
      prefs::kMessageCenterDisabledSystemComponentIds,
      &disabled_system_component_ids_);

  disabled_extension_id_pref_.Init(
      prefs::kMessageCenterDisabledExtensionIds,
      profile_->GetPrefs(),
      base::Bind(
          &NotifierStateTracker::OnStringListPrefChanged,
          base::Unretained(this),
          base::Unretained(prefs::kMessageCenterDisabledExtensionIds),
          base::Unretained(&disabled_extension_ids_)));

  disabled_system_component_id_pref_.Init(
      prefs::kMessageCenterDisabledSystemComponentIds,
      profile_->GetPrefs(),
      base::Bind(
          &NotifierStateTracker::OnStringListPrefChanged,
          base::Unretained(this),
          base::Unretained(prefs::kMessageCenterDisabledSystemComponentIds),
          base::Unretained(&disabled_system_component_ids_)));

#if BUILDFLAG(ENABLE_EXTENSIONS)
  extension_registry_observer_.Add(
      extensions::ExtensionRegistry::Get(profile_));
#endif
}

NotifierStateTracker::~NotifierStateTracker() {
}

bool NotifierStateTracker::IsNotifierEnabled(
    const NotifierId& notifier_id) const {
  switch (notifier_id.type) {
    case NotifierId::APPLICATION:
      return disabled_extension_ids_.find(notifier_id.id) ==
          disabled_extension_ids_.end();
    case NotifierId::WEB_PAGE:
      return PermissionManager::Get(profile_)
                 ->GetPermissionStatus(CONTENT_SETTINGS_TYPE_NOTIFICATIONS,
                                       notifier_id.url, notifier_id.url)
                 .content_setting == CONTENT_SETTING_ALLOW;
    case NotifierId::SYSTEM_COMPONENT:
#if defined(OS_CHROMEOS)
      return disabled_system_component_ids_.find(notifier_id.id) ==
          disabled_system_component_ids_.end();
#else
      // We do not disable system component notifications.
      return true;
#endif
    case NotifierId::ARC_APPLICATION:
#if defined(OS_CHROMEOS)
      // TODO(hriono): Ask Android if the application's notifications are
      // enabled.
      return true;
#else
      return false;
#endif
    default:
      NOTREACHED();
      break;
  }

  NOTREACHED();
  return false;
}

void NotifierStateTracker::SetNotifierEnabled(
    const NotifierId& notifier_id,
    bool enabled) {
  DCHECK_NE(NotifierId::WEB_PAGE, notifier_id.type);

  bool add_new_item = false;
  const char* pref_name = NULL;
  std::unique_ptr<base::StringValue> id;
  switch (notifier_id.type) {
    case NotifierId::APPLICATION:
      pref_name = prefs::kMessageCenterDisabledExtensionIds;
      add_new_item = !enabled;
      id.reset(new base::StringValue(notifier_id.id));
#if BUILDFLAG(ENABLE_EXTENSIONS)
      FirePermissionLevelChangedEvent(notifier_id, enabled);
#endif
      break;
    case NotifierId::SYSTEM_COMPONENT:
#if defined(OS_CHROMEOS)
      pref_name = prefs::kMessageCenterDisabledSystemComponentIds;
      add_new_item = !enabled;
      id.reset(new base::StringValue(notifier_id.id));
#else
      return;
#endif
      break;
    default:
      NOTREACHED();
  }
  DCHECK(pref_name != NULL);

  ListPrefUpdate update(profile_->GetPrefs(), pref_name);
  base::ListValue* const list = update.Get();
  if (add_new_item) {
    list->AppendIfNotPresent(std::move(id));
  } else {
    list->Remove(*id, nullptr);
  }
}

void NotifierStateTracker::OnStringListPrefChanged(
    const char* pref_name, std::set<std::string>* ids_field) {
  ids_field->clear();
  // Separate GetPrefs()->GetList() to analyze the crash. See crbug.com/322320
  const PrefService* pref_service = profile_->GetPrefs();
  CHECK(pref_service);
  const base::ListValue* pref_list = pref_service->GetList(pref_name);
  for (size_t i = 0; i < pref_list->GetSize(); ++i) {
    std::string element;
    if (pref_list->GetString(i, &element) && !element.empty())
      ids_field->insert(element);
    else
      LOG(WARNING) << i << "-th element is not a string for " << pref_name;
  }
}

#if BUILDFLAG(ENABLE_EXTENSIONS)
void NotifierStateTracker::OnExtensionUninstalled(
    content::BrowserContext* browser_context,
    const extensions::Extension* extension,
    extensions::UninstallReason reason) {
  NotifierId notifier_id(NotifierId::APPLICATION, extension->id());
  if (IsNotifierEnabled(notifier_id))
    return;

  SetNotifierEnabled(notifier_id, true);
}

void NotifierStateTracker::FirePermissionLevelChangedEvent(
    const NotifierId& notifier_id, bool enabled) {
  DCHECK_EQ(NotifierId::APPLICATION, notifier_id.type);
  extensions::api::notifications::PermissionLevel permission =
      enabled ? extensions::api::notifications::PERMISSION_LEVEL_GRANTED
              : extensions::api::notifications::PERMISSION_LEVEL_DENIED;
  std::unique_ptr<base::ListValue> args(new base::ListValue());
  args->AppendString(extensions::api::notifications::ToString(permission));
  std::unique_ptr<extensions::Event> event(new extensions::Event(
      extensions::events::NOTIFICATIONS_ON_PERMISSION_LEVEL_CHANGED,
      extensions::api::notifications::OnPermissionLevelChanged::kEventName,
      std::move(args)));
  extensions::EventRouter::Get(profile_)
      ->DispatchEventToExtension(notifier_id.id, std::move(event));

  // Tell the IO thread that this extension's permission for notifications
  // has changed.
  extensions::InfoMap* extension_info_map =
      extensions::ExtensionSystem::Get(profile_)->info_map();
  content::BrowserThread::PostTask(
      content::BrowserThread::IO, FROM_HERE,
      base::Bind(&extensions::InfoMap::SetNotificationsDisabled,
                 extension_info_map, notifier_id.id, !enabled));
}
#endif
