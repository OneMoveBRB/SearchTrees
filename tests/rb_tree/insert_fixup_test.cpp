#include "test_helpers.hpp"

#include <gtest/gtest.h>

#include <random>
#include <set>

using namespace search_trees;
using test::expect_matches_multiset;

namespace {

TEST(RBTreeInsertFixupTest, Case1RecolorPropagatesToRoot) {
  RBTree<int> tree;
  for (int v : {10, 5, 15, 1, 7})
    tree.insert(v);
  EXPECT_EQ(tree.size(), 5u);
  expect_matches_multiset(tree, std::multiset<int>{10, 5, 15, 1, 7});
}

TEST(RBTreeInsertFixupTest, Case2ThenCase3LeftSide) {
  RBTree<int> tree;
  for (int v : {10, 5, 7})
    tree.insert(v);
  EXPECT_EQ(tree.size(), 3u);
  expect_matches_multiset(tree, std::multiset<int>{10, 5, 7});
}

TEST(RBTreeInsertFixupTest, Case2ThenCase3RightSide) {
  RBTree<int> tree;
  for (int v : {10, 15, 12})
    tree.insert(v);
  EXPECT_EQ(tree.size(), 3u);
  expect_matches_multiset(tree, std::multiset<int>{10, 15, 12});
}

TEST(RBTreeInsertFixupTest, LLAndRRRotations) {
  RBTree<int> ll;
  for (int v : {30, 20, 10})
    ll.insert(v);
  expect_matches_multiset(ll, std::multiset<int>{30, 20, 10});

  RBTree<int> rr;
  for (int v : {10, 20, 30})
    rr.insert(v);
  expect_matches_multiset(rr, std::multiset<int>{10, 20, 30});
}

TEST(RBTreeInsertFixupTest, Case2ThenCase3Deep) {
  // More nodes, so that case 2 does not occur at the root.
  RBTree<int> tree;
  for (int v : {50, 25, 75, 10, 37, 60, 90, 30, 35})
    tree.insert(v);
  expect_matches_multiset(
      tree, std::multiset<int>{50, 25, 75, 10, 37, 60, 90, 30, 35});
}

TEST(RBTreeInsertFixupTest, AscendingInsertKeepsOrder) {
  RBTree<int> tree;
  std::multiset<int> expected;
  for (int i = 1; i <= 100; ++i) {
    tree.insert(i);
    expected.insert(i);
  }
  expect_matches_multiset(tree, expected);
}

TEST(RBTreeInsertFixupTest, DescendingInsertKeepsOrder) {
  RBTree<int> tree;
  std::multiset<int> expected;
  for (int i = 100; i >= 1; --i) {
    tree.insert(i);
    expected.insert(i);
  }
  expect_matches_multiset(tree, expected);
}

TEST(RBTreeInsertFixupTest, InsertAllPermutationsOfSmallSet) {
  std::vector<int> values = {1, 2, 3, 4, 5};
  std::sort(values.begin(), values.end());
  do {
    RBTree<int> tree;
    for (int v : values)
      tree.insert(v);
    expect_matches_multiset(tree,
                            std::multiset<int>(values.begin(), values.end()));
  } while (std::next_permutation(values.begin(), values.end()));
}

TEST(RBTreeInsertFixupTest, InsertRandomDuplicates) {
  RBTree<int> tree;
  std::multiset<int> expected;
  std::mt19937 rng(42);
  std::uniform_int_distribution<int> dist(0, 20);

  for (int i = 0; i < 1000; ++i) {
    int v = dist(rng);
    tree.insert(v);
    expected.insert(v);
  }
  expect_matches_multiset(tree, expected);
}

TEST(RBTreeInsertFixupTest, ManyZigZagInserts) {
  RBTree<int> tree;
  std::multiset<int> expected;
  for (int i = 0; i < 200; ++i) {
    int v = (i % 2 == 0) ? i : -i;
    tree.insert(v);
    expected.insert(v);
  }
  expect_matches_multiset(tree, expected);
}

}  // namespace
