#include <cstdlib>
#include <memory>

#include <UnorderedMap.h>
#include <string>
#include "gtest/gtest.h"

#include "Helper.h"

TEST(MapTests, Construct) {
  UnorderedMap<std::string, size_t> map;
  ASSERT_TRUE(map.empty());
}

TEST(MapTests, Insert) {
  static const size_t kIterations = 123456;
  UnorderedMap<std::string, size_t> map;
  for (size_t i = 0; i < kIterations; ++i) {
    map.insert({std::to_string(i), i});
  }

  ASSERT_EQ(map.size(), kIterations);
}

TEST(MapTests, find) {
  static const size_t kIterations = 123456;
  UnorderedMap<std::string, size_t> map;
  for (size_t i = 0; i < kIterations; ++i) {
    map.insert({std::to_string(i), i});
  }

  ASSERT_EQ(map.size(), kIterations);

  for (size_t i = 0; i < kIterations; ++i) {
    auto iter = map.find(std::to_string(i));
    ASSERT_EQ(iter->first, std::to_string(i));
    ASSERT_EQ(iter->second, i);
  }
}

TEST(MapTests, Iterator) {
  static const size_t kIterations = 123456;
  UnorderedMap<std::string, Helper> map;
  for (size_t i = 0; i < kIterations; ++i) {
    map.insert({std::to_string(i), Helper(i, i, i)});
  }

  std::vector<std::pair<std::string, Helper>> vector;
  for (const auto &item : map) {
    vector.emplace_back(item);
  }

  std::sort(vector.begin(), vector.end(),
            [](const std::pair<std::string, Helper> &first,
               const std::pair<std::string, Helper> &second) -> bool {
              return first.second.a_ < second.second.a_;
            });

  std::vector<size_t> sorted;
  for (size_t i = 0; i < kIterations; ++i) {
    sorted.push_back(vector[i].second.a_);
  }

  std::vector<size_t> right_sorted;
  for (size_t i = 0; i < kIterations; ++i) {
    right_sorted.push_back(i);
  }

  ASSERT_EQ(right_sorted, sorted);

  UnorderedMap<std::string, Helper> new_map;
  Helper::move_counter_ = 0;
  for (auto &&item : map) {
    new_map.insert(std::move(item));
  }
}
