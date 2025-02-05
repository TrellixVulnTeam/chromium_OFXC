/*
 * Copyright (C) 2004, 2006, 2009 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TextIterator_h
#define TextIterator_h

#include "core/CoreExport.h"
#include "core/dom/Range.h"
#include "core/editing/EphemeralRange.h"
#include "core/editing/FindOptions.h"
#include "core/editing/iterators/FullyClippedStateStack.h"
#include "core/editing/iterators/TextIteratorBehavior.h"
#include "core/editing/iterators/TextIteratorTextState.h"
#include "platform/heap/Handle.h"
#include "wtf/Vector.h"

namespace blink {

class InlineTextBox;
class LayoutText;
class LayoutTextFragment;

CORE_EXPORT String
plainText(const EphemeralRange&,
          const TextIteratorBehavior& = TextIteratorBehavior());

String plainText(const EphemeralRangeInFlatTree&,
                 const TextIteratorBehavior& = TextIteratorBehavior());

// Iterates through the DOM range, returning all the text, and 0-length
// boundaries at points where replaced elements break up the text flow.  The
// text comes back in chunks so as to optimize for performance of the iteration.

template <typename Strategy>
class CORE_TEMPLATE_CLASS_EXPORT TextIteratorAlgorithm {
  STACK_ALLOCATED();

 public:
  // [start, end] indicates the document range that the iteration should take
  // place within (both ends inclusive).
  TextIteratorAlgorithm(const PositionTemplate<Strategy>& start,
                        const PositionTemplate<Strategy>& end,
                        const TextIteratorBehavior& = TextIteratorBehavior());
  ~TextIteratorAlgorithm();

  bool atEnd() const { return !m_textState.positionNode() || m_shouldStop; }
  void advance();
  bool isInsideAtomicInlineElement() const;
  bool isInTextSecurityMode() const;

  EphemeralRangeTemplate<Strategy> range() const;
  Node* node() const;

  Document* ownerDocument() const;
  Node* currentContainer() const;
  int startOffsetInCurrentContainer() const;
  int endOffsetInCurrentContainer() const;
  PositionTemplate<Strategy> startPositionInCurrentContainer() const;
  PositionTemplate<Strategy> endPositionInCurrentContainer() const;

  const TextIteratorTextState& text() const { return m_textState; }
  int length() const { return m_textState.length(); }
  UChar characterAt(unsigned index) const {
    return m_textState.characterAt(index);
  }

  bool breaksAtReplacedElement() {
    return !m_behavior.doesNotBreakAtReplacedElement();
  }

  // Calculate the minimum |actualLength >= minLength| such that code units
  // with offset range [position, position + actualLength) are whole code
  // points. Append these code points to |output| and return |actualLength|.
  // TODO(xiaochengh): Use (start, end) instead of (start, length).
  int copyTextTo(ForwardsTextBuffer* output, int position, int minLength) const;
  // TODO(xiaochengh): Avoid default parameters.
  int copyTextTo(ForwardsTextBuffer* output, int position = 0) const;

  // Computes the length of the given range using a text iterator. The default
  // iteration behavior is to always emit object replacement characters for
  // replaced elements. When |forSelectionPreservation| is set to true, it
  // also emits spaces for other non-text nodes using the
  // |TextIteratorEmitsCharactersBetweenAllVisiblePosition| mode.
  static int rangeLength(const PositionTemplate<Strategy>& start,
                         const PositionTemplate<Strategy>& end,
                         bool forSelectionPreservation = false);

  static bool shouldEmitTabBeforeNode(Node*);
  static bool shouldEmitNewlineBeforeNode(Node&);
  static bool shouldEmitNewlineAfterNode(Node&);
  static bool shouldEmitNewlineForNode(Node*, bool emitsOriginalText);

  static bool supportsAltText(Node*);

 private:
  enum IterationProgress {
    HandledNone,
    HandledOpenShadowRoots,
    HandledUserAgentShadowRoot,
    HandledNode,
    HandledChildren
  };

  void initialize(Node* startContainer,
                  int startOffset,
                  Node* endContainer,
                  int endOffset);

  void exitNode();
  bool shouldRepresentNodeOffsetZero();
  bool shouldEmitSpaceBeforeAndAfterNode(Node*);
  void representNodeOffsetZero();
  bool handleTextNode();
  bool handleReplacedElement();
  bool handleNonTextNode();
  void handleTextBox();
  void handleTextNodeFirstLetter(LayoutTextFragment*);
  // Helper function during initialization. Returns true if the start position
  // is in a text node with first-letter, in which case it also sets up related
  // parameters. Returns false otherwise.
  bool prepareForFirstLetterInitialization();
  bool hasNotAdvancedToStartPosition();
  int adjustedStartForFirstLetter(const Node&, const LayoutText&, int, int);
  int adjustedStartForRemainingText(const Node&, const LayoutText&, int, int);
  void spliceBuffer(UChar,
                    Node* textNode,
                    Node* offsetBaseNode,
                    int textStartOffset,
                    int textEndOffset);
  void emitText(Node* textNode,
                LayoutText* layoutObject,
                int textStartOffset,
                int textEndOffset);
  size_t restoreCollapsedTrailingSpace(InlineTextBox* nextTextBox,
                                       size_t subrunEnd);
  unsigned restoreCollapsedLeadingSpace(unsigned runStart);

  // Used by selection preservation code. There should be one character emitted
  // between every VisiblePosition in the Range used to create the TextIterator.
  // FIXME <rdar://problem/6028818>: This functionality should eventually be
  // phased out when we rewrite moveParagraphs to not clone/destroy moved
  // content.
  bool emitsCharactersBetweenAllVisiblePositions() const {
    return m_behavior.emitsCharactersBetweenAllVisiblePositions();
  }

  bool entersTextControls() const { return m_behavior.entersTextControls(); }

  // Used in pasting inside password field.
  bool emitsOriginalText() const { return m_behavior.emitsOriginalText(); }

  // Used when the visibility of the style should not affect text gathering.
  bool ignoresStyleVisibility() const {
    return m_behavior.ignoresStyleVisibility();
  }

  // Used when the iteration should stop if form controls are reached.
  bool stopsOnFormControls() const { return m_behavior.stopsOnFormControls(); }

  bool emitsImageAltText() const { return m_behavior.emitsImageAltText(); }

  bool entersOpenShadowRoots() const {
    return m_behavior.entersOpenShadowRoots();
  }

  bool emitsObjectReplacementCharacter() const {
    return m_behavior.emitsObjectReplacementCharacter();
  }

  bool excludesAutofilledValue() const {
    return m_behavior.excludeAutofilledValue();
  }

  bool doesNotBreakAtReplacedElement() const {
    return m_behavior.doesNotBreakAtReplacedElement();
  }

  bool forInnerText() const { return m_behavior.forInnerText(); }

  bool isBetweenSurrogatePair(int position) const;

  // Append code units with offset range [position, position + copyLength)
  // to the output buffer.
  void copyCodeUnitsTo(ForwardsTextBuffer* output,
                       int position,
                       int copyLength) const;

  // Current position, not necessarily of the text being returned, but position
  // as we walk through the DOM tree.
  Member<Node> m_node;
  int m_offset;
  IterationProgress m_iterationProgress;
  FullyClippedStateStackAlgorithm<Strategy> m_fullyClippedStack;
  int m_shadowDepth;

  // The range.
  Member<Node> m_startContainer;
  int m_startOffset;
  Member<Node> m_endContainer;
  int m_endOffset;
  // |m_endNode| stores |Strategy::childAt(*m_endContainer, m_endOffset - 1)|,
  // if it exists, or |nullptr| otherwise.
  Member<Node> m_endNode;
  Member<Node> m_pastEndNode;

  // Used when there is still some pending text from the current node; when
  // these are false and 0, we go back to normal iterating.
  bool m_needsAnotherNewline;
  InlineTextBox* m_textBox;
  // Used when iteration over :first-letter text to save pointer to
  // remaining text box.
  InlineTextBox* m_remainingTextBox;
  // Used to point to LayoutText object for :first-letter.
  LayoutText* m_firstLetterText;

  // Used to do the whitespace collapsing logic.
  Member<Text> m_lastTextNode;
  bool m_lastTextNodeEndedWithCollapsedSpace;

  // Used when text boxes are out of order (Hebrew/Arabic w/ embeded LTR text)
  Vector<InlineTextBox*> m_sortedTextBoxes;
  size_t m_sortedTextBoxesPosition;

  const TextIteratorBehavior m_behavior;

  // Used when deciding text fragment created by :first-letter should be looked
  // into.
  bool m_handledFirstLetter;
  // Used when stopsOnFormControls() is true to determine if the iterator should
  // keep advancing.
  bool m_shouldStop;
  // Used for use counter |InnerTextWithShadowTree| and
  // |SelectionToStringWithShadowTree|, we should not use other purpose.
  bool m_handleShadowRoot;

  // Used for adjusting the initialization and the output when the start
  // container is a text node with :first-letter.
  int m_firstLetterStartOffset;
  int m_remainingTextStartOffset;

  // Contains state of emitted text.
  TextIteratorTextState m_textState;
};

extern template class CORE_EXTERN_TEMPLATE_EXPORT
    TextIteratorAlgorithm<EditingStrategy>;
extern template class CORE_EXTERN_TEMPLATE_EXPORT
    TextIteratorAlgorithm<EditingInFlatTreeStrategy>;

using TextIterator = TextIteratorAlgorithm<EditingStrategy>;
using TextIteratorInFlatTree = TextIteratorAlgorithm<EditingInFlatTreeStrategy>;

}  // namespace blink

#endif  // TextIterator_h
