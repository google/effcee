// Copyright 2017 The Effcee Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef EFFCEE_CURSOR_H
#define EFFCEE_CURSOR_H

#include <sstream>
#include <string>

#include "export.h"
#include "re2/stringpiece.h"

namespace effcee {

using StringPiece = re2::StringPiece;

// Represents a position in a StringPiece, while tracking line number.
class Cursor {
 public:
  EFFCEE_EXPORT explicit Cursor(StringPiece str)
      : remaining_(str), line_num_(1) {}

  EFFCEE_EXPORT StringPiece remaining() const { return remaining_; }
  // Returns the current 1-based line number.
  EFFCEE_EXPORT int line_num() const { return line_num_; }

  // Returns true if the remaining text is empty.
  EFFCEE_EXPORT bool Exhausted() const { return remaining_.empty(); }

  // Returns a string piece from the current position until the end of the line
  // or the end of input, up to and including the newline.
  EFFCEE_EXPORT StringPiece RestOfLine() const {
    const auto newline_pos = remaining_.find('\n');
    return remaining_.substr(0,
                             newline_pos + (newline_pos != StringPiece::npos));
  }

  // Advance |n| characters.  Does not adjust line count.  The next |n|
  // characters should not contain newlines if line numbering is to remain
  // up to date.  Returns this object.
  EFFCEE_EXPORT Cursor& Advance(size_t n) {
    remaining_.remove_prefix(n);
    return *this;
  }

  // Advances the cursor by a line.  If no text remains, then does nothing.
  // Otherwise removes the first line (including newline) and increments the
  // line count.  If there is no newline then the remaining string becomes
  // empty.  Returns this object.
  EFFCEE_EXPORT Cursor& AdvanceLine() {
    if (remaining_.size()) {
      Advance(RestOfLine().size());
      ++line_num_;
    }
    return *this;
  }

 private:
  // The remaining text, after all previous advancements.  References the
  // original string storage.
  StringPiece remaining_;
  // The current 1-based line number.
  int line_num_;
};

// Returns string containing a description of the line containing a given
// subtext, with a message, and a caret displaying the subtext position.
// Assumes subtext does not contain a newline.
EFFCEE_EXPORT
inline std::string LineMessage(StringPiece text, StringPiece subtext,
                               StringPiece message) {
  Cursor c(text);
  StringPiece full_line = c.RestOfLine();
  const auto* subtext_end = subtext.data() + subtext.size();
  while (subtext_end - (full_line.data() + full_line.size()) > 0) {
    c.AdvanceLine();
    full_line = c.RestOfLine();
  }
  const char* full_line_newline =
      full_line.find('\n') == StringPiece::npos ? "\n" : "";
  const auto column = size_t(subtext.data() - full_line.data());

  std::ostringstream out;
  out << ":" << c.line_num() << ":" << (1 + column) << ": " << message << "\n"
      << full_line << full_line_newline << std::string(column, ' ') << "^\n";

  return out.str();
}

}  // namespace effcee

#endif
