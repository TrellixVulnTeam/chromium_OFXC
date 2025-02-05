// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_RESOURCES_SCOPED_RESOURCE_H_
#define CC_RESOURCES_SCOPED_RESOURCE_H_

#include <memory>

#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "cc/base/cc_export.h"
#include "cc/resources/resource.h"

#if DCHECK_IS_ON()
#include "base/threading/platform_thread.h"
#endif

namespace cc {

class CC_EXPORT ScopedResource : public Resource {
 public:
  explicit ScopedResource(ResourceProvider* provider);
  virtual ~ScopedResource();

  void Allocate(const gfx::Size& size,
                ResourceProvider::TextureHint hint,
                ResourceFormat format,
                const gfx::ColorSpace& color_space);
  void AllocateWithGpuMemoryBuffer(const gfx::Size& size,
                                   ResourceFormat format,
                                   gfx::BufferUsage usage,
                                   const gfx::ColorSpace& color_space);
  void Free();

 private:
  ResourceProvider* resource_provider_;

#if DCHECK_IS_ON()
  base::PlatformThreadId allocate_thread_id_;
#endif

  DISALLOW_COPY_AND_ASSIGN(ScopedResource);
};

}  // namespace cc

#endif  // CC_RESOURCES_SCOPED_RESOURCE_H_
