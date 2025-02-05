/*
 * Copyright (C) 2010, Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#ifndef DelayProcessor_h
#define DelayProcessor_h

#include "modules/webaudio/AudioParam.h"
#include "platform/audio/AudioDSPKernelProcessor.h"
#include "wtf/RefPtr.h"
#include <memory>

namespace blink {

class AudioDSPKernel;

class DelayProcessor final : public AudioDSPKernelProcessor {
 public:
  DelayProcessor(float sampleRate,
                 unsigned numberOfChannels,
                 AudioParamHandler& delayTime,
                 double maxDelayTime);
  ~DelayProcessor() override;

  std::unique_ptr<AudioDSPKernel> createKernel() override;

  void processOnlyAudioParams(size_t framesToProcess) override;

  AudioParamHandler& delayTime() const { return *m_delayTime; }
  double maxDelayTime() { return m_maxDelayTime; }

 private:
  RefPtr<AudioParamHandler> m_delayTime;
  double m_maxDelayTime;
};

}  // namespace blink

#endif  // DelayProcessor_h
