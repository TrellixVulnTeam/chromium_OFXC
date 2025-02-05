// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_CONTENT_SECURITY_POLICY_CONTENT_SECURITY_POLICY_H_
#define CONTENT_COMMON_CONTENT_SECURITY_POLICY_CONTENT_SECURITY_POLICY_H_

#include <memory>
#include <vector>

#include "content/common/content_export.h"
#include "content/common/content_security_policy/csp_directive.h"
#include "content/common/content_security_policy_header.h"
#include "url/gurl.h"

namespace content {

class CSPContext;

// https://www.w3.org/TR/CSP3/#framework-policy
//
// A ContentSecurityPolicy is a collection of CSPDirectives which will be
// enforced upon requests.
struct CONTENT_EXPORT ContentSecurityPolicy {
  ContentSecurityPolicy();
  ContentSecurityPolicy(blink::WebContentSecurityPolicyType disposition,
                        blink::WebContentSecurityPolicySource source,
                        const std::vector<CSPDirective>& directives,
                        const std::vector<std::string>& report_endpoints,
                        const std::string& header);
  ContentSecurityPolicy(const ContentSecurityPolicy&);
  ~ContentSecurityPolicy();

  blink::WebContentSecurityPolicyType disposition;
  blink::WebContentSecurityPolicySource source;
  std::vector<CSPDirective> directives;
  std::vector<std::string> report_endpoints;
  std::string header;

  std::string ToString() const;

  // Return true when the |policy| allows a request to the |url| in relation to
  // the |directive| for a given |context|.
  // Note: Any policy violation are reported to the |context|.
  static bool Allow(const ContentSecurityPolicy& policy,
                    CSPDirective::Name directive,
                    const GURL& url,
                    CSPContext* context,
                    bool is_redirect = false);
};

}  // namespace content
#endif  // CONTENT_COMMON_CONTENT_SECURITY_POLICY_CONTENT_SECURITY_POLICY_H_
