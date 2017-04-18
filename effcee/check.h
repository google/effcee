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

#ifndef EFFCEE_CHECK_H
#define EFFCEE_CHECK_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "effcee.h"
#include "make_unique.h"

namespace effcee {

// A single check indicating something to be matched.
//
// A _positive_ check is _resolved_ when its parameter is matches a part of the
// in the input text.  A _negative_ check is _resolved_ when its parameter does
// _not_ match a section of the input between context-dependent start and end
// points.
class Check {
 public:
  // The type Determines when the check is satisfied.  The Not type denotes
  // a negative check.  The other types denote positive checks.
  enum class Type {
    Simple,  // Matches a string.
    Next,    // Matches a string, on the line following previous match.
    Same,    // Matches a string, on the same line as the previous metch.
    DAG,     // Matches a string, unordered with respect to other
    Label,   // Like Simple, but resets local variables.
    Not,     // Given string is not found before next positive match.
  };

  // A Part is a contiguous segment of the check pattern.  A part is
  // distinguished by how it matches against input.
  class Part {
   public:
    // A Constraint describes what part of the input string is matched.
    enum class Constraint {
      Substring,  // Can skip some leading characters before matching.
      Prefix,     // Match the beginning of the string.
    };

    enum class Type {
      Fixed,  // A fixed string: characters are matched exactly, in sequence.
    };

    Part(Constraint constraint, Type type, StringPiece param)
        : constraint_(constraint), type_(type), param_(param) {}

    // Returns a new Part with the given parameters.
    static std::unique_ptr<Part> MakePart(Constraint constraint, Type type,
                                          StringPiece param);

    // Tries to match the specified portion of the given string. If successful,
    // returns true, advances |input| past the matched portion, and saves the
    // captured substring in captured. Otherwise returns false and does not
    // update |input| or |captured|.
    bool Matches(StringPiece* input, StringPiece* captured) const;

   private:
    // The constraint on what part of the input should match.
    Constraint constraint_;
    // The part type.
    Type type_;
    // The part parameter.
    StringPiece param_;
  };

  using Parts = std::vector<std::unique_ptr<Part>>;

  // MSVC needs a default constructor.  However, a default-constructed Check
  // instance can't be used for matching.
  Check() {}

  // Construct a Check object of the given type and fixed parameter string.
  // In particular, this retains a StringPiece reference to the |param|
  // contents, so that string storage should remain valid for the duration
  // of this object.
  Check(Type type, StringPiece param) : type_(type), param_(param) {
    parts_.push_back(
        Part::MakePart(Part::Constraint::Substring, Part::Type::Fixed, param));
  }

  // Move constructor.
  Check(Check&& other) : type_(other.type_), param_(other.param_) {
    parts_.swap(other.parts_);
  }
  // Copy constructor.
  Check(const Check& other) : type_(other.type_), param_(other.param_) {
    for (const auto& part : other.parts_) {
      parts_.push_back(make_unique<Part>(*part));
    }
  }
  // Copy and move assignment.
  Check& operator=(Check other) {
    type_ = other.type_;
    param_ = other.param_;
    std::swap(parts_, other.parts_);
    return *this;
  }

  // Accessors.
  Type type() const { return type_; }
  StringPiece param() const { return param_; }

  // Tries to match the given string. If successful, returns true, advances
  // |str| past the matched portion, and saves the captured substring in captured.
  // Otherwise returns false and does not update |str| or |captured|.  Assumes
  // this instance is not default-constructed.
  bool Matches(StringPiece* str, StringPiece* captured) const;

  // Returns a string describing this check.
  std::string Description(const Options& options) const;

 private:
  // The type of check.
  Type type_;

  // The parameter as given in user input, if any.
  StringPiece param_;

  // The parameter, broken down into parts.
  Parts parts_;
};

// Equality operator for Check.
inline bool operator==(const Check& lhs, const Check& rhs) {
  return lhs.type() == rhs.type() && lhs.param() == rhs.param();
}

// Inequality operator for Check.
inline bool operator!=(const Check& lhs, const Check& rhs) {
  return !(lhs == rhs);
}

using CheckList = std::vector<Check>;

// Parses |checks_string|, returning a Result status object and the sequence
// of recognized checks, taking |options| into account.  The result status
// object indicates success, or failure with a message.
// TODO(dneto): Only matches simple checks for now.
std::pair<Result, CheckList> ParseChecks(StringPiece checks_string,
                                         const Options& options);

}  // namespace effcee

#endif
