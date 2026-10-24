#include "test_helpers.hpp"

#include <gtest/gtest.h>

#include <random>
#include <set>

using namespace search_trees;
using test::expect_matches_multiset;

namespace {

TEST(RBTreeEraseTest, EraseFromEmptyReturnsFalse) {
  RBTree<int> tree;
  EXPECT_FALSE(tree.erase(1));
}

TEST(RBTreeEraseTest, EraseMissingKeyReturnsFalse) {
  RBTree<int> tree;
  for (int v : {5, 3, 7})
    tree.insert(v);
  EXPECT_FALSE(tree.erase(4));
  EXPECT_EQ(tree.size(), 3u);
}

TEST(RBTreeEraseTest, EraseDecrementsKeyCountBeforeNodeRemoval) {
  RBTree<int> tree;
  tree.insert(5);
  tree.insert(5);
  tree.insert(5);

  EXPECT_TRUE(tree.erase(5));
  EXPECT_EQ(tree.size(), 2u);
  EXPECT_EQ(tree.count(5), 2u);

  EXPECT_TRUE(tree.erase(5));
  EXPECT_EQ(tree.size(), 1u);
  EXPECT_EQ(tree.count(5), 1u);

  EXPECT_TRUE(tree.erase(5));
  EXPECT_EQ(tree.size(), 0u);
  EXPECT_EQ(tree.count(5), 0u);
  EXPECT_TRUE(tree.empty());
}

TEST(RBTreeEraseTest, EraseNodeWithNoChildren) {
  RBTree<int> tree;
  for (int v : {10, 5, 15})
    tree.insert(v);
  EXPECT_TRUE(tree.erase(5));
  expect_matches_multiset(tree, std::multiset<int>{10, 15});
}

TEST(RBTreeEraseTest, EraseNodeWithOnlyLeftChild) {
  RBTree<int> tree;
  for (int v : {10, 5, 3})
    tree.insert(v);
  EXPECT_TRUE(tree.erase(5));
  expect_matches_multiset(tree, std::multiset<int>{10, 3});
}

TEST(RBTreeEraseTest, EraseNodeWithOnlyRightChild) {
  RBTree<int> tree;
  for (int v : {10, 5, 8})
    tree.insert(v);
  EXPECT_TRUE(tree.erase(5));
  expect_matches_multiset(tree, std::multiset<int>{10, 8});
}

TEST(RBTreeEraseTest, EraseNodeWithTwoChildrenUsesSuccessor) {
  RBTree<int> tree;
  for (int v : {20, 10, 30, 25, 35, 22, 27})
    tree.insert(v);
  EXPECT_TRUE(tree.erase(20));
  expect_matches_multiset(tree, std::multiset<int>{10, 30, 25, 35, 22, 27});
}

TEST(RBTreeEraseTest, EraseRootWithTwoChildren) {
  RBTree<int> tree;
  for (int v : {20, 10, 30})
    tree.insert(v);
  EXPECT_TRUE(tree.erase(20));
  expect_matches_multiset(tree, std::multiset<int>{10, 30});
  EXPECT_EQ(tree.lower_bound(0), 10);
}

TEST(RBTreeEraseTest, EraseRootRepeatedlyUntilEmpty) {
  RBTree<int> tree;
  for (int v : {50, 30, 70, 20, 40, 60, 80})
    tree.insert(v);
  for (int v : {50, 30, 70, 20, 40, 60, 80}) {
    EXPECT_TRUE(tree.erase(v));
  }
  EXPECT_TRUE(tree.empty());
}

TEST(RBTreeEraseTest, EraseAllInRandomOrder) {
  std::vector<int> values;
  for (int i = 0; i < 100; ++i)
    values.push_back(i);

  std::mt19937 rng(7);
  std::shuffle(values.begin(), values.end(), rng);

  RBTree<int> tree;
  for (int v : values)
    tree.insert(v);
  EXPECT_EQ(tree.size(), 100u);

  for (int v : values) {
    EXPECT_TRUE(tree.erase(v));
  }
  EXPECT_TRUE(tree.empty());
}

TEST(RBTreeEraseTest, EraseRandomDuplicatesOneAtATime) {
  RBTree<int> tree;
  std::multiset<int> expected;

  std::mt19937 rng(123);
  std::uniform_int_distribution<int> dist(0, 10);
  for (int i = 0; i < 500; ++i) {
    int v = dist(rng);
    tree.insert(v);
    expected.insert(v);
  }

  for (int i = 0; i < 500; ++i) {
    int v = dist(rng);
    bool tree_res = tree.erase(v);
    auto it = expected.find(v);
    bool oracle_res = it != expected.end();
    EXPECT_EQ(tree_res, oracle_res);
    if (oracle_res)
      expected.erase(it);
    EXPECT_EQ(tree.size(), expected.size());
  }

  expect_matches_multiset(tree, expected);
}

TEST(RBTreeEraseTest, EraseAbsentAfterAbsent) {
  RBTree<int> tree;
  for (int i = 0; i < 10; ++i)
    tree.insert(i * 10);

  for (int i = 0; i < 10; ++i) {
    EXPECT_FALSE(tree.erase(i * 10 + 5));
  }
  EXPECT_EQ(tree.size(), 10u);
}

TEST(RBTreeEraseTest, EraseRightChain) {
  RBTree<int> tree;
  for (int i = 0; i < 50; ++i)
    tree.insert(i);
  for (int i = 0; i < 50; ++i)
    EXPECT_TRUE(tree.erase(i));
  EXPECT_TRUE(tree.empty());
}

TEST(RBTreeEraseTest, EraseLeftChain) {
  RBTree<int> tree;
  for (int i = 49; i >= 0; --i)
    tree.insert(i);
  for (int i = 49; i >= 0; --i)
    EXPECT_TRUE(tree.erase(i));
  EXPECT_TRUE(tree.empty());
}

TEST(RBTreeEraseTest, EraseAlternating) {
  RBTree<int> tree;
  for (int i = 0; i < 100; ++i)
    tree.insert(i);

  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(tree.erase(i));
    EXPECT_TRUE(tree.erase(99 - i));
  }
  EXPECT_TRUE(tree.empty());
}

}  // namespace
