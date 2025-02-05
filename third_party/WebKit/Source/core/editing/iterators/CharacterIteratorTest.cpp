/*
 * Copyright (c) 2013, Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "core/editing/iterators/CharacterIterator.h"

#include "core/editing/EditingTestBase.h"
#include "core/frame/FrameView.h"

namespace blink {

class CharacterIteratorTest : public EditingTestBase {};

TEST_F(CharacterIteratorTest, SubrangeWithReplacedElements) {
  static const char* bodyContent =
      "<div id='div' contenteditable='true'>1<img src='foo.png'>345</div>";
  setBodyContent(bodyContent);
  document().view()->updateAllLifecyclePhases();

  Node* divNode = document().getElementById("div");
  Range* entireRange = Range::create(document(), divNode, 0, divNode, 3);

  EphemeralRange result =
      calculateCharacterSubrange(EphemeralRange(entireRange), 2, 3);
  Node* textNode = divNode->lastChild();
  EXPECT_EQ(Position(textNode, 0), result.startPosition());
  EXPECT_EQ(Position(textNode, 3), result.endPosition());
}

TEST_F(CharacterIteratorTest, CollapsedSubrange) {
  static const char* bodyContent =
      "<div id='div' contenteditable='true'>hello</div>";
  setBodyContent(bodyContent);
  document().view()->updateAllLifecyclePhases();

  Node* textNode = document().getElementById("div")->lastChild();
  Range* entireRange = Range::create(document(), textNode, 1, textNode, 4);
  EXPECT_EQ(1u, entireRange->startOffset());
  EXPECT_EQ(4u, entireRange->endOffset());

  const EphemeralRange& result =
      calculateCharacterSubrange(EphemeralRange(entireRange), 2, 0);
  EXPECT_EQ(Position(textNode, 3), result.startPosition());
  EXPECT_EQ(Position(textNode, 3), result.endPosition());
}

}  // namespace blink
