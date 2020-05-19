#include <Differentiator.h>
#include "gtest/gtest.h"

class Tests : public ::testing::Test {
 public:
  Tests()
      : variables_({{{"x", "1"}, {"y", "1"}, {"z", "1"}},
                    {{"x", "2"}, {"y", "2"}, {"z", "2"}},
                    {{"x", "5.5"}, {"y", "2.2"}, {"z", "9.9"}},
                    {{"x", "1001"}, {"y", "0.1234"}, {"z", "123"}},
                    {{"x", "1524.23"}, {"y", "763.1"}, {"z", "1.4723"}}}) {}

 protected:
  Differentiator differentiator_;
  Vector<UnorderedMap<String, String>> variables_;
  String expr_;
  static const size_t kPoints = 5;
};

TEST_F(Tests, Test_1) {
  expr_ = "0";
  Vector<String> answers = {"0", "0", "0", "0", "0"};
  for (size_t i = 0; i < Tests::kPoints; ++i) {
    EXPECT_EQ(std::stold(answers[i]),
              std::stold(differentiator_.Differentiate(expr_, "x")
                             .At(variables_[i])
                             .ToString()));
  }
}

TEST_F(Tests, Test_2) {
  expr_ = "x*x/2";
  Vector<String> answers = {"1", "2", "5.5", "1001", "1524.23"};
  for (size_t i = 0; i < Tests::kPoints; ++i) {
    EXPECT_EQ(std::stold(answers[i]),
              std::stold(differentiator_.Differentiate(expr_, "x")
                             .At(variables_[i])
                             .ToString()));
  }
}

TEST_F(Tests, Test_3) {
  expr_ = "(y*z*x) * (1 + 2 + 3) + x*x*x*x + (y+x) * (z - x / (z + x)) * x";
  Vector<String> answers = {"11", "64", "920.378", "4012260000", "14164800000"};
  for (size_t i = 0; i < Tests::kPoints; ++i) {
    EXPECT_LE(
        std::abs(1 - std::stold(answers[i]) /
                         std::stold(differentiator_.Differentiate(expr_, "x")
                                        .At(variables_[i])
                                        .ToString())),
        0.00001);
  }
}

TEST_F(Tests, Test_4) {
  expr_ =
      "(x + y) / (x + z) + (x + z) / (x + y) + (y + x) / (y + z) + (y + z) / "
      "(y + x) + (z + x) / (z + y) + (z + y) / (z + x)";
  Vector<String> answers = {"0", "0", "-0.187215", "0.0159982", "0.00195963"};
  for (size_t i = 0; i < Tests::kPoints; ++i) {
    EXPECT_LE(std::abs(std::stold(answers[i]) -
                       std::stold(differentiator_.Differentiate(expr_, "x")
                                      .At(variables_[i])
                                      .ToString())),
              std::abs(0.001 * std::stold(answers[i])));
  }
}

TEST_F(Tests, Test_5) {
  expr_ = "log(x^x)";
  Vector<String> answers = {"1", "1.693147180559945309", "2.70475",
                            "7.9087547793152205852", "8.32924"};
  for (size_t i = 0; i < Tests::kPoints; ++i) {
    EXPECT_LE(std::abs(std::stold(answers[i]) -
                       std::stold(differentiator_.Differentiate(expr_, "x")
                                      .At(variables_[i])
                                      .ToString())),
              std::abs(0.00001 * std::stold(answers[i])));
  }
}

TEST_F(Tests, Test_6) {
  expr_ = "log(x^cos(x)*y^sin(x))";
  Vector<String> answers = {"0.54030230586813971", "-1.126801372419797793015",
                            "1.89037", "-5.53631", "-1.75464"};
  for (size_t i = 0; i < Tests::kPoints; ++i) {
    EXPECT_LE(std::abs(std::stold(answers[i]) -
                       std::stold(differentiator_.Differentiate(expr_, "x")
                                      .At(variables_[i])
                                      .ToString())),
              std::abs(0.0001 * std::stold(answers[i])));
  }
}