#pragma once

#include <cstddef>

template <typename Key>
class RBTree {
 public:
 private:
  enum class Color {
    Red = 0,
    Black = 1
  };

  struct BaseNode {
    BaseNode()
        : left(this),
          right(this),
          parent(this),
          color(Color::Black),
          subtree_size(0) {}

    BaseNode(BaseNode* nil, Color color, std::size_t subtree_size)
        : left(nil),
          right(nil),
          parent(nil),
          color(color),
          subtree_size(subtree_size) {}

    BaseNode* left;
    BaseNode* right;
    BaseNode* parent;
    Color color;
    std::size_t subtree_size;
  };

  struct Node : BaseNode {
    Node(const Key& key, BaseNode* nil)
        : BaseNode(nil, Color::Red, 1), key(key), key_cnt(1) {}

    Key key;
    std::size_t key_cnt;
  };

  BaseNode* nil_;
  BaseNode* root_;
};
