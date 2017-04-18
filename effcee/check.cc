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

#include "check.h"

#include <cassert>
#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "cursor.h"
#include "effcee.h"

using Constraint = effcee::Check::Part::Constraint;
using Status = effcee::Result::Status;
using StringPiece = effcee::StringPiece;
using Type = effcee::Check::Type;

namespace {

// Returns a table of suffix to type mappings.
const std::vector<std::pair<StringPiece, Type>>& TypeStringTable() {
  static std::vector<std::pair<StringPiece, Type>> type_str_table{
      {"", Type::Simple},  {"-NEXT", Type::Next},   {"-SAME", Type::Same},
      {"-DAG", Type::DAG}, {"-LABEL", Type::Label}, {"-NOT", Type::Not}};
  return type_str_table;
}

// Returns the Check::Type value matching the suffix part of a check rule
// prefix.  Assumes |suffix| is valid.
Type TypeForSuffix(StringPiece suffix) {
  const auto& type_str_table = TypeStringTable();
  const auto pair_iter =
      std::find_if(type_str_table.begin(), type_str_table.end(),
                   [suffix](const std::pair<StringPiece, Type>& elem) {
                     return suffix == elem.first;
                   });
  assert(pair_iter != type_str_table.end());
  return pair_iter->second;
}

StringPiece SuffixForType(Type type) {
  const auto& type_str_table = TypeStringTable();
  const auto pair_iter =
      std::find_if(type_str_table.begin(), type_str_table.end(),
                   [type](const std::pair<StringPiece, Type>& elem) {
                     return type == elem.second;
                   });
  assert(pair_iter != type_str_table.end());
  return pair_iter->first;
}
}  // namespace

namespace effcee {

std::unique_ptr<Check::Part> Check::Part::MakePart(
    Check::Part::Constraint constraint, Check::Part::Type type,
    StringPiece param) {
  return std::unique_ptr<Check::Part>(new Check::Part(constraint, type, param));
}

Check::Check(Type type, StringPiece param) : type_(type), param_(param) {
  parts_.push_back(
      Part::MakePart(Part::Constraint::Substring, Part::Type::Fixed, param));
}

bool Check::Part::Matches(StringPiece* str, StringPiece* captured) const {
  // TODO(dneto): Handle other constraints and types.
  assert(type_ == Type::Fixed);
  switch (constraint_) {
    case Constraint::Prefix:
      if (!str->starts_with(param_)) return false;
      *captured = str->substr(0, param_.size());
      str->remove_prefix(param_.size());
      break;
    case Constraint::Substring: {
      auto where = str->find(param_);
      if (where == StringPiece::npos) {
        return false;
      } else {
        *captured = str->substr(where, param_.size());
        str->remove_prefix(where + param_.size());
      }
    } break;
  }
  return true;
}

std::string Check::Part::Regex() {
  switch (type_) {
    case Type::Fixed:
      return RE2::QuoteMeta(param_);
    case Type::Regex:
      return std::string(param_.data(), param_.size());
  }
}

bool Check::Matches(StringPiece* input, StringPiece* captured) const {
  // This is not valid to call on default-constructed instance.
  assert(!parts_.empty());

  std::ostringstream consume_regex;
  consume_regex << "(";
  for (auto& part : parts_) {
    consume_regex << part->Regex();
  }
  consume_regex << ")";

  return RE2::FindAndConsume(input, consume_regex.str(), captured);
}

std::string Check::Description(const Options& options) const {
  std::ostringstream out;
  out << options.prefix() << SuffixForType(type()) << ": " << param();
  return out.str();
}

std::pair<Result, CheckList> ParseChecks(StringPiece str,
                                         const Options& options) {
  // Returns a pair whose first member is a result constructed from the
  // given status and message, and the second member is an empy pattern.
  auto failure = [](Status status, StringPiece message) {
    return std::make_pair(Result(status, message), CheckList{});
  };

  if (options.prefix().size() == 0)
    return failure(Status::BadOption, "Rule prefix is empty");
  if (RE2::FullMatch(options.prefix(), "\\s+"))
    return failure(Status::BadOption,
                   "Rule prefix is whitespace.  That's silly.");

  CheckList check_list;

  const auto quoted_prefix = RE2::QuoteMeta(options.prefix());
  // Match the following parts:
  //    .*?               - Text that is not the rule prefix
  //    quoted_prefix     - A Simple Check prefix
  //    (-NEXT|-SAME)?    - An optional check type suffix. Two shown here.
  //    :                 - Colon
  //    \s*               - Whitespace
  //    (.*?)             - Captured parameter
  //    \s*               - Whitespace
  //    $                 - End of line

  const RE2 regexp(std::string(".*?") + quoted_prefix +
                   "(-NEXT|-SAME|-DAG|-LABEL|-NOT)?"
                   ":\\s*(.*?)\\s*$");
  Cursor cursor(str);
  while (!cursor.Exhausted()) {
    const auto line = cursor.RestOfLine();

    StringPiece matched_param;
    StringPiece suffix;
    if (RE2::PartialMatch(line, regexp, &suffix, &matched_param)) {
      const Type type = TypeForSuffix(suffix);

      Check::Parts parts;
      StringPiece param = matched_param;
      StringPiece fixed, regex;
      auto constraint = [&parts]() {
        return parts.empty() ? Check::Part::Constraint::Substring
                             : Check::Part::Constraint::Prefix;
      };
      while (RE2::Consume(&param, "(.*?){{(.*?)}}", &fixed, &regex)) {
        if (!fixed.empty()) {
          parts.push_back(Check::Part::MakePart(
              constraint(), Check::Part::Type::Fixed, fixed));
        }
        if (!regex.empty()) {
          parts.push_back(Check::Part::MakePart(
              constraint(), Check::Part::Type::Regex, regex));
        }
      }
      if (!param.empty()) {
        parts.push_back(Check::Part::MakePart(constraint(),
                                              Check::Part::Type::Fixed, param));
      }

      check_list.push_back(Check(type, matched_param, std::move(parts)));
    }
    cursor.AdvanceLine();
  }

  if (check_list.empty()) {
    return failure(
        Status::NoRules,
        std::string("No check rules specified. Looking for prefix ") +
            options.prefix());
  }
  if (check_list[0].type() == Type::Same) {
    return failure(Status::BadRule, std::string(options.prefix()) +
                                        "-SAME can't be the first check rule");
  }

  return std::make_pair(Result(Result::Status::Ok), check_list);
}
}  // namespace effcee
