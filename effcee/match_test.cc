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

#include "gmock/gmock.h"

#include "effcee.h"

namespace {

using effcee::Match;
using effcee::Options;
using ::testing::Eq;
using ::testing::HasSubstr;

const char* kNotFound = "error: expected string not found in input";
const char* kMissedSame =
    "error: CHECK-SAME: is not on the same line as previous match";
const char* kNextOnSame =
    "error: CHECK-NEXT: is on the same line as previous match";
const char* kNextTooLate =
    "error: CHECK-NEXT: is not on the line after the previous match";
const char* kNotStrFound = "error: CHECK-NOT: string occurred!";

// Match free function

TEST(Match, FreeFunctionLinks) {
  Match("", "");
  Match("", "", effcee::Options());
}

// Simple checks

TEST(Match, OneSimpleCheckPass) {
  const auto result = Match("Hello", "CHECK: Hello");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, OneSimpleCheckFail) {
  const auto result = Match("World", "CHECK: Hello");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK: Hello"));
}

TEST(Match, TwoSimpleChecksPass) {
  const auto result = Match("Hello\nWorld", "CHECK: Hello\nCHECK: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, TwoSimpleChecksPassWithSurroundingText) {
  const auto input = R"(Say
                        Hello
                        World
                        Today)";
  const auto result = Match(input, "CHECK: Hello\nCHECK: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, TwoSimpleChecksPassWithInterveningText) {
  const auto input = R"(Hello
                        Between
                        World)";
  const auto result = Match(input, "CHECK: Hello\nCHECK: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, TwoSimpleChecksPassWhenInSequenceSameLine) {
  const auto result = Match("HelloWorld", "CHECK: Hello\nCHECK: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, TwoSimpleChecksFailWhenReversed) {
  const auto result = Match("HelloWorld", "CHECK: World\nCHECK: Hello");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK: Hello"));
}

TEST(Match, SimpleThenSamePasses) {
  const auto result = Match("HelloWorld", "CHECK: Hello\nCHECK-SAME: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, SimpleThenSamePassesWithInterveningOnSameLine) {
  const auto result = Match("Hello...World", "CHECK: Hello\nCHECK-SAME: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, SimpleThenSameFailsIfOnNextLine) {
  const auto result = Match("Hello\nWorld", "CHECK: Hello\nCHECK-SAME: World");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(),HasSubstr(kMissedSame));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-SAME: World"));
}

TEST(Match, SimpleThenSameFailsIfOnMuchLaterLine) {
  const auto result =
      Match("Hello\n\nz\n\nWorld", "CHECK: Hello\nCHECK-SAME: World");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kMissedSame));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-SAME: World"));
}

TEST(Match, SimpleThenSameFailsIfNeverMatched) {
  const auto result = Match("Hello\nHome", "CHECK: Hello\nCHECK-SAME: World");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-SAME: World"));
}

TEST(Match, SimpleThenNextOnSameLineFails) {
  const auto result = Match("HelloWorld", "CHECK: Hello\nCHECK-NEXT: World");
  EXPECT_FALSE(result);
  EXPECT_THAT(result.message(), HasSubstr(kNextOnSame));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-NEXT: World"));
}

TEST(Match, SimpleThenNextPassesIfOnNextLine) {
  const auto result = Match("Hello\nWorld", "CHECK: Hello\nCHECK-NEXT: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, SimpleThenNextFailsIfOnAfterNextLine) {
  const auto result = Match("Hello\nfoo\nWorld", "CHECK: Hello\nCHECK-NEXT: World");
  EXPECT_THAT(result.message(), HasSubstr(kNextTooLate));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-NEXT: World"));
}

TEST(Match, SimpleThenNextFailsIfNeverMatched) {
  const auto result =
      Match("Hello\nHome", "CHECK: Hello\nCHECK-NEXT: World");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-NEXT: World"));
}

// TODO: CHECK-NOT

TEST(Match, AloneNotNeverSeenPasses) {
  const auto result = Match("Hello", "CHECK-NOT: Borg");
  EXPECT_TRUE(result);
}

TEST(Match, LeadingNotNeverSeenPasses) {
  const auto result = Match("Hello", "CHECK-NOT: Borg\nCHECK: Hello");
  EXPECT_TRUE(result);
}

TEST(Match, BetweenNotNeverSeenPasses) {
  const auto result =
      Match("HelloWorld", "CHECK: Hello\nCHECK-NOT: Borg\nCHECK: World");
  EXPECT_TRUE(result);
}

TEST(Match, BetweenNotDotsNeverSeenPasses) {
  // The before and after matches occur on the same line.
  const auto result =
      Match("Hello...World", "CHECK: Hello\nCHECK-NOT: Borg\nCHECK: World");
  EXPECT_TRUE(result);
}

TEST(Match, BetweenNotLinesNeverSeenPasses) {
  // The before and after matches occur on different lines.
  const auto result =
      Match("Hello\nz\nWorld", "CHECK: Hello\nCHECK-NOT: Borg\nCHECK: World");
  EXPECT_TRUE(result);
}

TEST(Match, TrailingNotNeverSeenPasses) {
  const auto result = Match("Hello", "CHECK: Hello\nCHECK-NOT: Borg");
  EXPECT_TRUE(result);
}

TEST(Match, AloneNotSeenFails) {
  const auto result = Match("Borg", "CHECK-NOT: Borg");
  EXPECT_FALSE(result);
  EXPECT_THAT(result.message(), HasSubstr(kNotStrFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-NOT: Borg"));
}

TEST(Match, LeadingNotSeenFails) {
  const auto result = Match("Borg", "CHECK-NOT: Borg\nCHECK: Hello");
  EXPECT_FALSE(result);
  EXPECT_THAT(result.message(), HasSubstr(kNotStrFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-NOT: Borg"));
}

TEST(Match, BetweenNotSeenFails) {
  const auto result =
      Match("HelloBorgWorld", "CHECK: Hello\nCHECK-NOT: Borg\nCHECK: World");
  EXPECT_FALSE(result);
  EXPECT_THAT(result.message(), HasSubstr(kNotStrFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-NOT: Borg"));
}

TEST(Match, BetweenNotDotsSeenFails) {
  const auto result =
      Match("Hello.Borg.World", "CHECK: Hello\nCHECK-NOT: Borg\nCHECK: World");
  EXPECT_FALSE(result);
  EXPECT_THAT(result.message(), HasSubstr(kNotStrFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-NOT: Borg"));
}

TEST(Match, BetweenNotLinesSeenFails) {
  const auto result = Match("Hello\nBorg\nWorld",
                            "CHECK: Hello\nCHECK-NOT: Borg\nCHECK: World");
  EXPECT_FALSE(result);
  EXPECT_THAT(result.message(), HasSubstr(kNotStrFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-NOT: Borg"));
}

TEST(Match, TrailingNotSeenFails) {
  const auto result = Match("HelloBorg", "CHECK: Hello\nCHECK-NOT: Borg");
  EXPECT_FALSE(result);
  EXPECT_THAT(result.message(), HasSubstr(kNotStrFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-NOT: Borg"));
}

// WIP: CHECK-LABEL

TEST(Match, OneLabelCheckPass) {
  const auto result = Match("Hello", "CHECK-LABEL: Hello");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, OneLabelCheckFail) {
  const auto result = Match("World", "CHECK-LABEL: Hello");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-LABEL: Hello"));
}

TEST(Match, TwoLabelChecksPass) {
  const auto result =
      Match("Hello\nWorld", "CHECK-LABEL: Hello\nCHECK-LABEL: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, TwoLabelChecksPassWithSurroundingText) {
  const auto input = R"(Say
                        Hello
                        World
                        Today)";
  const auto result = Match(input, "CHECK-LABEL: Hello\nCHECK-LABEL: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, TwoLabelChecksPassWithInterveningText) {
  const auto input = R"(Hello
                        Between
                        World)";
  const auto result = Match(input, "CHECK-LABEL: Hello\nCHECK-LABEL: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, TwoLabelChecksPassWhenInSequenceSameLine) {
  const auto result =
      Match("HelloWorld", "CHECK-LABEL: Hello\nCHECK-LABEL: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, TwoLabelChecksFailWhenReversed) {
  const auto result =
      Match("HelloWorld", "CHECK-LABEL: World\nCHECK-LABEL: Hello");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-LABEL: Hello"));
}

// WIP: Mixture of Simple and Label checks

TEST(Match, SimpleAndLabelChecksPass) {
  const auto result = Match("Hello\nWorld", "CHECK: Hello\nCHECK-LABEL: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, LabelAndSimpleChecksPass) {
  const auto result = Match("Hello\nWorld", "CHECK-LABEL: Hello\nCHECK: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, SimpleAndLabelChecksFails) {
  const auto result = Match("Hello\nWorld", "CHECK: Hello\nCHECK-LABEL: Band");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-LABEL: Band"));
}

TEST(Match, LabelAndSimpleChecksFails) {
  const auto result = Match("Hello\nWorld", "CHECK-LABEL: Hello\nCHECK: Band");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK: Band"));
}

// DAG checks: Part 1: Tests simlar to simple checks tests

TEST(Match, OneDAGCheckPass) {
  const auto result = Match("Hello", "CHECK-DAG: Hello");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, OneDAGCheckFail) {
  const auto result = Match("World", "CHECK-DAG: Hello");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-DAG: Hello"));
}

TEST(Match, TwoDAGChecksPass) {
  const auto result = Match("Hello\nWorld", "CHECK-DAG: Hello\nCHECK-DAG: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, TwoDAGChecksPassWithSurroundingText) {
  const auto input = R"(Say
                        Hello
                        World
                        Today)";
  const auto result = Match(input, "CHECK-DAG: Hello\nCHECK-DAG: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, TwoDAGChecksPassWithInterveningText) {
  const auto input = R"(Hello
                        Between
                        World)";
  const auto result = Match(input, "CHECK-DAG: Hello\nCHECK-DAG: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, TwoDAGChecksPassWhenInSequenceSameLine) {
  const auto result = Match("HelloWorld", "CHECK-DAG: Hello\nCHECK-DAG: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, DAGThenSamePasses) {
  const auto result = Match("HelloWorld", "CHECK-DAG: Hello\nCHECK-SAME: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, DAGThenSamePassesWithInterveningOnSameLine) {
  const auto result = Match("Hello...World", "CHECK-DAG: Hello\nCHECK-SAME: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, DAGThenSameFailsIfOnNextLine) {
  const auto result = Match("Hello\nWorld", "CHECK-DAG: Hello\nCHECK-SAME: World");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kMissedSame));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-SAME: World"));
}

TEST(Match, DAGThenSameFailsIfOnMuchLaterLine) {
  const auto result =
      Match("Hello\n\nz\n\nWorld", "CHECK-DAG: Hello\nCHECK-SAME: World");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kMissedSame));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-SAME: World"));
}

TEST(Match, DAGThenSameFailsIfNeverMatched) {
  const auto result = Match("Hello\nHome", "CHECK-DAG: Hello\nCHECK-SAME: World");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-SAME: World"));
}

TEST(Match, DAGThenNextOnSameLineFails) {
  const auto result = Match("HelloWorld", "CHECK-DAG: Hello\nCHECK-NEXT: World");
  EXPECT_FALSE(result);
  EXPECT_THAT(result.message(), HasSubstr(kNextOnSame));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-NEXT: World"));
}

TEST(Match, DAGThenNextPassesIfOnNextLine) {
  const auto result = Match("Hello\nWorld", "CHECK-DAG: Hello\nCHECK-NEXT: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, DAGThenNextPassesIfOnAfterNextLine) {
  const auto result = Match("Hello\nWorld", "CHECK-DAG: Hello\nCHECK-NEXT: World");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, DAGThenNextFailsIfNeverMatched) {
  const auto result =
      Match("Hello\nHome", "CHECK-DAG: Hello\nCHECK-NEXT: World");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-NEXT: World"));
}

// DAG checks: Part 2: Out of order matching

TEST(Match, TwoDAGMatchedOutOfOrderPasses) {
  const auto result = Match("Hello\nWorld", "CHECK-DAG: World\nCHECK-DAG: Hello");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, ThreeDAGMatchedOutOfOrderPasses) {
  const auto result =
      Match("Hello\nWorld\nNow",
            "CHECK-DAG: Now\nCHECK-DAG: World\nCHECK-DAG: Hello");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, TwoDAGChecksPassWhenReversedMatchingSameLine) {
  const auto result = Match("HelloWorld", "CHECK-DAG: World\nCHECK-DAG: Hello");
  EXPECT_TRUE(result) << result.message();
}

TEST(Match, DAGChecksGreedilyConsumeInput) {
  const auto result =
      Match("Hello\nBlocker\nWorld\n",
            "CHECK-DAG: Hello\nCHECK-DAG: World\nCHECK: Blocker");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-DAG: World"));
}

// DAG checks: Part 3: Interaction with Not checks

TEST(Match, DAGsAreSeparatedByNot) {
  // In this case the search for "Before" consumes the entire input.
  const auto result =
      Match("After\nBlocker\nBefore\n",
            "CHECK-DAG: Before\nCHECK-NOT: nothing\nCHECK-DAG: After");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-DAG: After"));
}

TEST(Match, TwoDAGsAreSeparatedByNot) {
  const auto result = Match("After\nApres\nBlocker\nBefore\nAnte",
                            "CHECK-DAG: Ante\nCHECK-DAG: Before\nCHECK-NOT: "
                            "nothing\nCHECK-DAG: Apres\nCHECK-DAG: After");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-DAG: Apres"));
}

// DAG checks: Part 4: Interaction with simple checks

TEST(Match, DAGsAreTerminatedBySimple) {
  const auto result =
      Match("After\nsimple\nBefore\n",
            "CHECK-DAG: Before\nCHECK: simple\nCHECK-DAG: After");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-DAG: Before"));
}

TEST(Match, TwoDAGsAreTerminatedBySimple) {
  const auto result = Match("After\nApres\nBlocker\nBefore\nAnte",
                            "CHECK-DAG: Ante\nCHECK-DAG: Before\nCHECK: "
                            "Blocker\nCHECK-DAG: Apres\nCHECK-DAG: After");
  EXPECT_FALSE(result) << result.message();
  EXPECT_THAT(result.message(), HasSubstr(kNotFound));
  EXPECT_THAT(result.message(), HasSubstr("CHECK-DAG: Ante"));
}

// Test detailed message text

TEST(Match, MessageStringNotFoundWhenNeverMatchedAnything) {
  const char* input = R"(Begin
Hello
 World)";
  const char* checks = R"(
Hello
  ;  CHECK: Needle
)";
  const char* expected = R"(chklist:3:13: error: expected string not found in input
  ;  CHECK: Needle
            ^
myin.txt:1:1: note: scanning from here
Begin
^
)";
  const auto result =
      Match(input, checks,
            Options().SetInputName("myin.txt").SetChecksName("chklist"));
  EXPECT_FALSE(result);
  EXPECT_THAT(result.message(), Eq(expected)) << result.message();
}

TEST(Match, MessageStringNotFoundAfterInitialMatch) {
  const char* input = R"(Begin
Hello
 World)";
  const char* checks = R"(
Hello
  ;  CHECK-LABEL: Hel
  ;  CHECK: Needle
)";
  const char* expected = R"(chklist:4:13: error: expected string not found in input
  ;  CHECK: Needle
            ^
myin.txt:2:4: note: scanning from here
Hello
   ^
)";
  const auto result =
      Match(input, checks,
            Options().SetInputName("myin.txt").SetChecksName("chklist"));
  EXPECT_FALSE(result);
  EXPECT_THAT(result.message(), Eq(expected)) << result.message();
}

TEST(Match, MessageCheckNotStringFoundAtStart) {
  const auto result =
      Match("  Cheese", "CHECK-NOT: Cheese",
            Options().SetInputName("in").SetChecksName("checks"));
  EXPECT_FALSE(result);
  const char* expected = R"(in:1:3: error: CHECK-NOT: string occurred!
  Cheese
  ^
checks:1:12: note: CHECK-NOT: pattern specified here
CHECK-NOT: Cheese
           ^
)";
  EXPECT_THAT(result.message(), Eq(expected)) << result.message();
}

TEST(Match, MessageCheckNotStringFoundAfterInitialMatch) {
  const auto result =
      Match("Cream    Cheese", "CHECK: Cream\nCHECK-NOT: Cheese",
            Options().SetInputName("in").SetChecksName("checks"));
  EXPECT_FALSE(result);
  const char* expected = R"(in:1:10: error: CHECK-NOT: string occurred!
Cream    Cheese
         ^
checks:2:12: note: CHECK-NOT: pattern specified here
CHECK-NOT: Cheese
           ^
)";
  EXPECT_THAT(result.message(), Eq(expected)) << result.message();
}

TEST(Match, MessageCheckSameFails) {
  const char* input = R"(
Bees
Make
Delicious Honey
)";
  const char* checks = R"(
CHECK: Make
CHECK-SAME: Honey
)";

  const auto result = Match(
      input, checks, Options().SetInputName("in").SetChecksName("checks"));
  EXPECT_FALSE(result);
  const char* expected = R"(checks:3:13: error: CHECK-SAME: is not on the same line as previous match
CHECK-SAME: Honey
            ^
in:4:11: note: 'next' match was here
Delicious Honey
          ^
in:3:5: note: previous match ended here
Make
    ^
)";
  EXPECT_THAT(result.message(), Eq(expected)) << result.message();
}

TEST(Match, MessageCheckNextFailsSinceOnSameLine) {
  const char* input = R"(
Bees
Make
Delicious Honey
)";
  const char* checks = R"(
CHECK: Bees
CHECK-NEXT: Honey
)";

  const auto result = Match(
      input, checks, Options().SetInputName("in").SetChecksName("checks"));
  EXPECT_FALSE(result);
  const char* expected = R"(checks:3:13: error: CHECK-NEXT: is not on the line after the previous match
CHECK-NEXT: Honey
            ^
in:4:11: note: 'next' match was here
Delicious Honey
          ^
in:2:5: note: previous match ended here
Bees
    ^
in:3:1: note: non-matching line after previous match is here
Make
^
)";
  EXPECT_THAT(result.message(), Eq(expected)) << result.message();
}

TEST(Match, MessageCheckNextFailsSinceLaterLine) {
  const char* input = R"(
Bees Make Delicious Honey
)";
  const char* checks = R"(
CHECK: Make
CHECK-NEXT: Honey
)";

  const auto result = Match(
      input, checks, Options().SetInputName("in").SetChecksName("checks"));
  EXPECT_FALSE(result);
  const char* expected = R"(checks:3:13: error: CHECK-NEXT: is on the same line as previous match
CHECK-NEXT: Honey
            ^
in:2:21: note: 'next' match was here
Bees Make Delicious Honey
                    ^
in:2:10: note: previous match ended here
Bees Make Delicious Honey
         ^
)";
  EXPECT_THAT(result.message(), Eq(expected)) << result.message();
}

TEST(Match, MessageUnresolvedDAG) {
  const char* input = R"(
Bees
Make
Delicious Honey
)";
  const char* checks = R"(
CHECK: ees
CHECK-DAG: Flowers
CHECK: Honey
)";

  const auto result = Match(
      input, checks, Options().SetInputName("in").SetChecksName("checks"));
  EXPECT_FALSE(result);
  const char* expected = R"(checks:3:12: error: expected string not found in input
CHECK-DAG: Flowers
           ^
in:2:5: note: scanning from here
Bees
    ^
in:4:11: note: next check matches here
Delicious Honey
          ^
)";
  EXPECT_THAT(result.message(), Eq(expected)) << result.message();
}


// TODO: Regexp
// TODO: Stateful

}  // namespace
