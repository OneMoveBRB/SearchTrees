#include "test_helpers.hpp"

#include <gtest/gtest.h>

#include <random>
#include <set>

using namespace search_trees;
using test::expect_matches_multiset;

namespace {

TEST(RBTreeDeleteFixupTest, Case1RedSibling) {
  RBTree<int> tree;
  for (int v : {10, 5, 15, 20})
    tree.insert(v);
  tree.erase(5);
  expect_matches_multiset(tree, std::multiset<int>{10, 15, 20});
}

TEST(RBTreeDeleteFixupTest, Case2BlackSiblingBlackChildren) {
  RBTree<int> tree;
  for (int v : {10, 5, 15, 3, 7})
    tree.insert(v);
  tree.erase(3);
  tree.erase(7);
  expect_matches_multiset(tree, std::multiset<int>{10, 5, 15});
}

TEST(RBTreeDeleteFixupTest, Case3AndCase4RotationsLeft) {
  RBTree<int> tree;
  for (int v : {10, 5, 20, 15, 25, 12})
    tree.insert(v);
  tree.erase(5);
  expect_matches_multiset(tree, std::multiset<int>{10, 20, 15, 25, 12});
}

TEST(RBTreeDeleteFixupTest, Case3AndCase4RotationsRight) {
  RBTree<int> tree;
  for (int v : {10, 20, 5, 3, 7, 8})
    tree.insert(v);
  tree.erase(20);
  expect_matches_multiset(tree, std::multiset<int>{10, 5, 3, 7, 8});
}

TEST(RBTreeDeleteFixupTest, SuccessorIsDirectRightChild) {
  RBTree<int> tree;
  for (int v : {20, 10, 30})
    tree.insert(v);
  tree.erase(20);
  expect_matches_multiset(tree, std::multiset<int>{10, 30});
}

TEST(RBTreeDeleteFixupTest, SuccessorIsDeepRightDescendant) {
  RBTree<int> tree;
  for (int v : {20, 10, 40, 30, 50, 25, 35})
    tree.insert(v);
  tree.erase(20);
  expect_matches_multiset(tree, std::multiset<int>{10, 40, 30, 50, 25, 35});
}

TEST(RBTreeDeleteFixupTest, DeleteBlackLeafWithBlackSiblingRedFarChild) {
  RBTree<int> tree;
  for (int v : {20, 10, 30, 25, 35, 40})
    tree.insert(v);
  tree.erase(10);
  expect_matches_multiset(tree, std::multiset<int>{20, 30, 25, 35, 40});
}

TEST(RBTreeDeleteFixupTest, DeleteBlackLeafWithBlackSiblingRedNearChild) {
  RBTree<int> tree;
  for (int v : {20, 10, 30, 25, 35, 27})
    tree.insert(v);
  tree.erase(10);
  expect_matches_multiset(tree, std::multiset<int>{20, 30, 25, 35, 27});
}

TEST(RBTreeDeleteFixupTest, DeleteFixupPropagatesToRoot) {
  RBTree<int> tree;
  for (int i = 0; i < 20; ++i)
    tree.insert(i);
  for (int i = 0; i < 10; ++i)
    tree.erase(i);
  std::multiset<int> expected;
  for (int i = 10; i < 20; ++i)
    expected.insert(i);
  expect_matches_multiset(tree, expected);
}

TEST(RBTreeDeleteFixupTest, DeleteAllInAscendingOrderAfterBigTree) {
  RBTree<int> tree;
  std::multiset<int> expected;
  for (int i = 0; i < 200; ++i) {
    tree.insert(i);
    expected.insert(i);
  }

  for (int i = 0; i < 200; ++i) {
    ASSERT_TRUE(tree.erase(i));
    expected.erase(expected.find(i));
    ASSERT_EQ(tree.size(), expected.size());
  }
  EXPECT_TRUE(tree.empty());
}

TEST(RBTreeDeleteFixupTest, DeleteAllInDescendingOrderAfterBigTree) {
  RBTree<int> tree;
  std::multiset<int> expected;
  for (int i = 0; i < 200; ++i) {
    tree.insert(i);
    expected.insert(i);
  }

  for (int i = 199; i >= 0; --i) {
    ASSERT_TRUE(tree.erase(i));
    expected.erase(expected.find(i));
    ASSERT_EQ(tree.size(), expected.size());
  }
  EXPECT_TRUE(tree.empty());
}

TEST(RBTreeDeleteFixupTest, DeleteAllInRandomOrderAfterBigTree) {
  std::vector<int> values(200);
  std::iota(values.begin(), values.end(), 0);

  RBTree<int> tree;
  for (int v : values)
    tree.insert(v);

  std::mt19937 rng(999);
  std::shuffle(values.begin(), values.end(), rng);

  for (int v : values) {
    ASSERT_TRUE(tree.erase(v));
    EXPECT_EQ(tree.size(),
              static_cast<std::size_t>(&v - &values[0]) + 1 == values.size() ?
                  0u :
                  tree.size());
  }
  EXPECT_TRUE(tree.empty());
}

TEST(RBTreeDeleteFixupTest, EraseRepeatedlyFromLargeTree) {
  RBTree<int> tree;
  std::multiset<int> expected;
  for (int i = 0; i < 100; ++i) {
    tree.insert(i);
    expected.insert(i);
  }

  std::mt19937 rng(1);
  std::uniform_int_distribution<int> dist(0, 99);

  for (int step = 0; step < 300; ++step) {
    int v = dist(rng);
    bool tree_res = tree.erase(v);
    auto it = expected.find(v);
    bool oracle_res = it != expected.end();
    EXPECT_EQ(tree_res, oracle_res);
    if (oracle_res)
      expected.erase(it);
    EXPECT_EQ(tree.size(), expected.size());
    expect_matches_multiset(tree, expected);
  }
}

}  // namespace
