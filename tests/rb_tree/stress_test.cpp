#include "test_helpers.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <set>

using namespace search_trees;
using test::expect_matches_multiset;
using test::to_sorted_vector;

namespace {

TEST(RBTreeStressTest, RandomizedAgainstMultiset) {
  std::mt19937 rng(12345);
  std::uniform_int_distribution<int> key_dist(-1000, 1000);
  std::bernoulli_distribution op_dist(0.5);

  RBTree<int> tree;
  std::multiset<int> oracle;

  constexpr int kOperations = 5000;
  for (int i = 0; i < kOperations; ++i) {
    const int key = key_dist(rng);
    if (op_dist(rng)) {
      tree.insert(key);
      oracle.insert(key);
    } else {
      const bool tree_res = tree.erase(key);
      auto it = oracle.find(key);
      const bool oracle_res = it != oracle.end();
      if (oracle_res)
        oracle.erase(it);  // Erase exactly one occurrence.
      ASSERT_EQ(tree_res, oracle_res) << "erase result mismatch for " << key;
    }

    ASSERT_EQ(tree.size(), oracle.size());

    if (i % 100 == 0) {
      expect_matches_multiset(tree, oracle);

      auto lb_tree = tree.lower_bound(key);
      auto lb_oracle = oracle.lower_bound(key);
      if (lb_oracle == oracle.end()) {
        ASSERT_EQ(lb_tree, std::nullopt);
      } else {
        ASSERT_TRUE(lb_tree.has_value());
        ASSERT_EQ(*lb_tree, *lb_oracle);
      }

      const auto expected_less = static_cast<std::size_t>(
          std::distance(oracle.begin(), oracle.lower_bound(key)));
      ASSERT_EQ(tree.count_less_than(key), expected_less);
    }
  }

  expect_matches_multiset(tree, oracle);
}

TEST(RBTreeStressTest, SmallRangeHighDuplicates) {
  std::mt19937 rng(7);
  std::uniform_int_distribution<int> key_dist(0, 5);
  std::bernoulli_distribution op_dist(0.5);

  RBTree<int> tree;
  std::multiset<int> oracle;

  for (int i = 0; i < 3000; ++i) {
    int key = key_dist(rng);
    if (op_dist(rng)) {
      tree.insert(key);
      oracle.insert(key);
    } else {
      bool tr = tree.erase(key);
      auto it = oracle.find(key);
      bool orr = it != oracle.end();
      if (orr)
        oracle.erase(it);
      ASSERT_EQ(tr, orr);
    }
    ASSERT_EQ(tree.size(), oracle.size());
    if (i % 50 == 0)
      expect_matches_multiset(tree, oracle);
  }
  expect_matches_multiset(tree, oracle);
}

TEST(RBTreeStressTest, InsertHeavyThenEraseHeavy) {
  std::mt19937 rng(2024);
  std::uniform_int_distribution<int> key_dist(-500, 500);

  RBTree<int> tree;
  std::multiset<int> oracle;

  for (int i = 0; i < 2000; ++i) {
    int key = key_dist(rng);
    tree.insert(key);
    oracle.insert(key);
  }
  expect_matches_multiset(tree, oracle);

  // Erase-heavy phase.
  for (int i = 0; i < 2000; ++i) {
    int key = key_dist(rng);
    bool tr = tree.erase(key);
    auto it = oracle.find(key);
    bool orr = it != oracle.end();
    if (orr)
      oracle.erase(it);
    ASSERT_EQ(tr, orr);
    ASSERT_EQ(tree.size(), oracle.size());
  }
  expect_matches_multiset(tree, oracle);
}

TEST(RBTreeStressTest, MixedSmallRangeSweep) {
  std::mt19937 rng(31);
  RBTree<int> tree;
  std::multiset<int> oracle;

  for (int round = 0; round < 50; ++round) {
    // Batch insert.
    for (int i = 0; i < 100; ++i) {
      int key = static_cast<int>(rng() % 200) - 100;
      tree.insert(key);
      oracle.insert(key);
    }
    expect_matches_multiset(tree, oracle);

    // Batch erase.
    for (int i = 0; i < 100; ++i) {
      int key = static_cast<int>(rng() % 200) - 100;
      bool tr = tree.erase(key);
      auto it = oracle.find(key);
      bool orr = it != oracle.end();
      if (orr)
        oracle.erase(it);
      ASSERT_EQ(tr, orr);
    }
    expect_matches_multiset(tree, oracle);
  }
}

TEST(RBTreeStressTest, KthSmallestConsistentWithSortedVector) {
  std::mt19937 rng(555);
  std::uniform_int_distribution<int> key_dist(0, 100);

  RBTree<int> tree;
  std::multiset<int> oracle;

  for (int i = 0; i < 500; ++i) {
    int key = key_dist(rng);
    tree.insert(key);
    oracle.insert(key);
  }

  auto sorted = to_sorted_vector(tree);
  std::vector<int> expected(oracle.begin(), oracle.end());
  ASSERT_EQ(sorted, expected);

  for (int i = 0; i < 100; ++i) {
    int key = key_dist(rng);
    tree.erase(key);
    auto it = oracle.find(key);
    if (it != oracle.end())
      oracle.erase(it);
  }

  auto sorted2 = to_sorted_vector(tree);
  std::vector<int> expected2(oracle.begin(), oracle.end());
  ASSERT_EQ(sorted2, expected2);
}

TEST(RBTreeStressTest, CountLessThanMatchesDistance) {
  std::mt19937 rng(88);
  std::uniform_int_distribution<int> key_dist(-100, 100);

  RBTree<int> tree;
  std::multiset<int> oracle;

  for (int i = 0; i < 300; ++i) {
    int key = key_dist(rng);
    tree.insert(key);
    oracle.insert(key);
  }

  for (int i = 0; i < 100; ++i) {
    int key = key_dist(rng);
    auto expected = static_cast<std::size_t>(
        std::distance(oracle.begin(), oracle.lower_bound(key)));
    ASSERT_EQ(tree.count_less_than(key), expected);
  }
}

}  // namespace
