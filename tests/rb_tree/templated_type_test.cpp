#include "test_helpers.hpp"

#include <gtest/gtest.h>

#include <string>

using namespace search_trees;

namespace {

TEST(RBTreeTemplatedTypeTest, StringKeys) {
  RBTree<std::string> tree;
  tree.insert("banana");
  tree.insert("apple");
  tree.insert("cherry");
  tree.insert("apple");

  EXPECT_EQ(tree.count("apple"), 2u);
  EXPECT_EQ(tree.lower_bound("b"), "banana");
  EXPECT_EQ(tree.kth_smallest(1), "apple");
  EXPECT_EQ(tree.kth_smallest(2), "apple");
  EXPECT_EQ(tree.kth_smallest(3), "banana");
  EXPECT_EQ(tree.kth_smallest(4), "cherry");

  EXPECT_TRUE(tree.erase("apple"));
  EXPECT_EQ(tree.count("apple"), 1u);
}

TEST(RBTreeTemplatedTypeTest, StringEmptyAndSingle) {
  RBTree<std::string> tree;
  tree.insert("");
  tree.insert("a");
  tree.insert("ab");

  EXPECT_EQ(tree.lower_bound(""), "");
  EXPECT_EQ(tree.lower_bound("a"), "a");
  EXPECT_EQ(tree.lower_bound("aa"), "ab");
  EXPECT_EQ(tree.lower_bound("z"), std::nullopt);
}

TEST(RBTreeTemplatedTypeTest, LongString) {
  RBTree<std::string> tree;
  std::string big(1000, 'x');
  tree.insert(big);
  EXPECT_EQ(tree.count(big), 1u);
  EXPECT_TRUE(tree.erase(big));
  EXPECT_TRUE(tree.empty());
}

TEST(RBTreeTemplatedTypeTest, ManyStrings) {
  RBTree<std::string> tree;
  std::vector<std::string> values;
  for (int i = 0; i < 100; ++i) {
    values.push_back("key_" + std::to_string(i));
  }

  for (const auto& s : values)
    tree.insert(s);
  EXPECT_EQ(tree.size(), 100u);

  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(tree.lower_bound("key_" + std::to_string(i)),
              "key_" + std::to_string(i));
  }
}

TEST(RBTreeTemplatedTypeTest, StringReplaceLikeOrdering) {
  RBTree<std::string> tree;
  for (const char* s : {"m", "b", "z", "a", "c"})
    tree.insert(s);
  EXPECT_EQ(tree.kth_smallest(1), "a");
  EXPECT_EQ(tree.kth_smallest(5), "z");
}

}  // namespace
