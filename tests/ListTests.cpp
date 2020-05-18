#include <cstdlib>

#include <List.h>
#include "Helper.h"
#include "gtest/gtest.h"

TEST(ListTests, Construct) {
  List<int> list;
  ASSERT_TRUE(list.IsEmpty());
}

TEST(ListTests, Push_and_PopFront) {
  static const size_t kIterations = 12345;
  List<size_t> list;
  for (size_t i = 0; i < kIterations; ++i) {
    list.PushFront(i);
  }

  ASSERT_EQ(list.Size(), kIterations);

  for (size_t i = kIterations; i > 0; --i) {
    ASSERT_EQ(list.PopFront(), i - 1);
  }

  ASSERT_TRUE(list.IsEmpty());
}

TEST(ListTests, Iterator) {
  static const size_t kIterations = 12345;
  List<Helper> list;
  for (size_t i = 0; i < kIterations; ++i) {
    list.PushFront(Helper(i, i, i));
  }

  size_t i = kIterations;
  for (const auto &item : list) {
    ASSERT_EQ(item.GetItem().a_, --i);
  }

  i = 0;
  List<Helper> new_list;
  Helper::move_counter_ = 0;
  for (auto &&item : list) {
    new_list.PushFront(std::move(item.GetItem()));
  }
  ASSERT_EQ(Helper::move_counter_, kIterations);
}

TEST(ListTests, CopyConstruct) {
  static const size_t kIterations = 12345;

  List<Helper> list1;
  for (size_t i = 0; i < kIterations; ++i) {
    list1.PushFront(Helper(i, i, i));
  }

  List<Helper> list2 = list1;

  auto iter2 = list2.begin();
  for (auto iter1 = list1.begin(); iter1 != list1.end() || iter2 != list2.end();
       ++iter1, ++iter2) {
    ASSERT_EQ(iter2->GetItem().a_, iter1->GetItem().a_);
  }
}