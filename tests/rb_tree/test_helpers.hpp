#pragma once

#include "search_trees/rb_tree.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <set>
#include <vector>

namespace search_trees::test {

// Returns all elements of the tree in ascending order via kth_smallest.
// Indirectly validates the correctness of subtree_size and key_count.
template <typename Key>
std::vector<Key> to_sorted_vector(const RBTree<Key>& tree) {
  std::vector<Key> out;
  const std::size_t n = tree.size();
  out.reserve(n);
  for (std::size_t i = 1; i <= n; ++i) {
    auto v = tree.kth_smallest(i);
    EXPECT_TRUE(v.has_value()) << "kth_smallest(" << i << ") returned nullopt";
    if (v)
      out.push_back(*v);
  }
  return out;
}

// Compares the tree against a multiset, treating the latter as an oracle model.
template <typename Key>
void expect_matches_multiset(const RBTree<Key>& tree,
                             const std::multiset<Key>& expected) {
  EXPECT_EQ(tree.size(), expected.size());

  std::vector<Key> expected_sorted(expected.begin(), expected.end());
  EXPECT_EQ(to_sorted_vector(tree), expected_sorted);

  for (const auto& k : expected) {
    EXPECT_EQ(tree.count(k), expected.count(k))
        << "count mismatch for key " << k;
  }
}

}  // namespace search_trees::test
