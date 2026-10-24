#include "test_helpers.hpp"

#include <gtest/gtest.h>

using namespace search_trees;

namespace {

TEST(RBTreeBasicTest, EmptyTreeInitialState) {
  RBTree<int> tree;
  EXPECT_TRUE(tree.empty());
  EXPECT_EQ(tree.size(), 0u);
  EXPECT_EQ(tree.count(0), 0u);
  EXPECT_EQ(tree.count(42), 0u);
  EXPECT_EQ(tree.lower_bound(0), std::nullopt);
  EXPECT_EQ(tree.lower_bound(42), std::nullopt);
  EXPECT_EQ(tree.kth_smallest(0), std::nullopt);
  EXPECT_EQ(tree.kth_smallest(1), std::nullopt);
  EXPECT_EQ(tree.count_less_than(0), 0u);
  EXPECT_EQ(tree.count_less_than(42), 0u);
}

TEST(RBTreeBasicTest, InsertSingleElement) {
  RBTree<int> tree;
  tree.insert(5);

  EXPECT_FALSE(tree.empty());
  EXPECT_EQ(tree.size(), 1u);
  EXPECT_EQ(tree.count(5), 1u);
  EXPECT_EQ(tree.count(4), 0u);
  EXPECT_EQ(tree.lower_bound(5), 5);
  EXPECT_EQ(tree.lower_bound(4), 5);
  EXPECT_EQ(tree.lower_bound(6), std::nullopt);
  EXPECT_EQ(tree.kth_smallest(1), 5);
}

TEST(RBTreeBasicTest, DuplicateInsertIncrementsCount) {
  RBTree<int> tree;
  tree.insert(7);
  tree.insert(7);
  tree.insert(7);

  EXPECT_EQ(tree.size(), 3u);
  EXPECT_EQ(tree.count(7), 3u);
  EXPECT_EQ(tree.kth_smallest(1), 7);
  EXPECT_EQ(tree.kth_smallest(2), 7);
  EXPECT_EQ(tree.kth_smallest(3), 7);
  EXPECT_EQ(tree.kth_smallest(4), std::nullopt);
}

TEST(RBTreeBasicTest, InsertDuplicateDoesNotCreateNewNode) {
  RBTree<int> tree;
  for (int i = 0; i < 10; ++i)
    tree.insert(1);

  EXPECT_EQ(tree.size(), 10u);
  EXPECT_EQ(tree.count(1), 10u);
  EXPECT_EQ(tree.kth_smallest(1), 1);
  EXPECT_EQ(tree.kth_smallest(10), 1);
  EXPECT_EQ(tree.lower_bound(0), 1);
  EXPECT_EQ(tree.lower_bound(2), std::nullopt);
}

TEST(RBTreeBasicTest, ClearResetsTree) {
  RBTree<int> tree;
  for (int v : {5, 3, 8, 1, 4, 7, 9})
    tree.insert(v);

  tree.clear();

  EXPECT_TRUE(tree.empty());
  EXPECT_EQ(tree.size(), 0u);
  EXPECT_EQ(tree.lower_bound(0), std::nullopt);
  EXPECT_EQ(tree.kth_smallest(1), std::nullopt);
  EXPECT_EQ(tree.count(5), 0u);

  tree.insert(42);
  EXPECT_EQ(tree.size(), 1u);
  EXPECT_EQ(tree.count(42), 1u);
  EXPECT_EQ(tree.kth_smallest(1), 42);
}

TEST(RBTreeBasicTest, ClearOnEmptyTreeIsSafe) {
  RBTree<int> tree;
  tree.clear();
  tree.clear();
  EXPECT_TRUE(tree.empty());

  tree.insert(1);
  EXPECT_EQ(tree.size(), 1u);
}

TEST(RBTreeBasicTest, ClearAfterManyOperations) {
  RBTree<int> tree;
  for (int i = 0; i < 500; ++i)
    tree.insert(i % 50);
  for (int i = 0; i < 100; ++i)
    tree.erase(i);

  tree.clear();
  EXPECT_TRUE(tree.empty());

  for (int i = 100; i < 200; ++i)
    tree.insert(i);
  EXPECT_EQ(tree.size(), 100u);
  EXPECT_EQ(tree.lower_bound(100), 100);
  EXPECT_EQ(tree.lower_bound(200), std::nullopt);
}

TEST(RBTreeBasicTest, EmptyAfterErasingAllElements) {
  RBTree<int> tree;
  tree.insert(1);
  EXPECT_TRUE(tree.erase(1));
  EXPECT_TRUE(tree.empty());
  EXPECT_EQ(tree.size(), 0u);

  tree.insert(2);
  EXPECT_EQ(tree.size(), 1u);
}

TEST(RBTreeBasicTest, EraseOnlyKeyTwice) {
  RBTree<int> tree;
  tree.insert(10);
  EXPECT_TRUE(tree.erase(10));
  EXPECT_FALSE(tree.erase(10));
  EXPECT_TRUE(tree.empty());
}

TEST(RBTreeBasicTest, SingleElementAllOperations) {
  RBTree<int> tree;
  tree.insert(42);

  EXPECT_EQ(tree.size(), 1u);
  EXPECT_FALSE(tree.empty());
  EXPECT_EQ(tree.count(42), 1u);
  EXPECT_EQ(tree.lower_bound(42), 42);
  EXPECT_EQ(tree.lower_bound(41), 42);
  EXPECT_EQ(tree.lower_bound(43), std::nullopt);
  EXPECT_EQ(tree.kth_smallest(1), 42);
  EXPECT_EQ(tree.kth_smallest(2), std::nullopt);
  EXPECT_EQ(tree.count_less_than(42), 0u);
  EXPECT_EQ(tree.count_less_than(43), 1u);
}

}  // namespace
