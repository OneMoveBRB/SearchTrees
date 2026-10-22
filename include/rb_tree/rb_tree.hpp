#pragma once

#include <cstddef>
#include <optional>
#include <stack>

template <typename Key>
class RBTree {
 public:
  RBTree() : nil_(), root_(&nil_) {}

  ~RBTree() { destroy(root_); }

  RBTree(const RBTree&) = delete;
  RBTree& operator=(const RBTree&) = delete;
  RBTree(RBTree&&) = delete;
  RBTree& operator=(RBTree&&) = delete;

  void insert(const Key& key) {
    BaseNode* parent_node = &nil_;
    BaseNode* current_node = root_;

    while (current_node != &nil_) {
      parent_node = current_node;
      Node* data_node = node_cast(current_node);

      if (key < data_node->key) {
        current_node = current_node->left;
      } else if (data_node->key < key) {
        current_node = current_node->right;
      } else {
        ++data_node->key_count;
        update_ancestors(current_node);
        return;
      }
    }

    Node* new_node = new Node(key, &nil_);
    new_node->parent = parent_node;

    if (parent_node == &nil_) {
      root_ = new_node;
    } else if (key < node_cast(parent_node)->key) {
      parent_node->left = new_node;
    } else {
      parent_node->right = new_node;
    }

    update_ancestors(parent_node);
    insert_fixup(new_node);
  }

  bool erase(const Key& key) {
    BaseNode* target_node = find_node(key);

    if (target_node == &nil_) {
      return false;
    }

    Node* target_data = node_cast(target_node);
    if (target_data->key_count > 1) {
      --target_data->key_count;
      update_ancestors(target_node);
      return true;
    }

    erase_node(target_node);
    return true;
  }

  std::optional<Key> lower_bound(const Key& key) const {
    const BaseNode* current_node = root_;
    const BaseNode* answer_node = &nil_;

    while (current_node != &nil_) {
      const Node* data_node = node_cast(current_node);

      if (!(data_node->key < key)) {
        answer_node = current_node;
        current_node = current_node->left;
      } else {
        current_node = current_node->right;
      }
    }

    if (answer_node == &nil_) {
      return std::nullopt;
    }

    return node_cast(answer_node)->key;
  }

  std::optional<Key> kth_smallest(std::size_t order) const {
    if (order == 0 || order > size()) {
      return std::nullopt;
    }

    const BaseNode* current_node = root_;

    while (current_node != &nil_) {
      const std::size_t left_size = current_node->left->subtree_size;
      const std::size_t key_count = node_cast(current_node)->key_count;

      if (order <= left_size) {
        current_node = current_node->left;
      } else if (order <= left_size + key_count) {
        return node_cast(current_node)->key;
      } else {
        order -= left_size + key_count;
        current_node = current_node->right;
      }
    }

    return std::nullopt;
  }

  std::size_t count(const Key& key) const {
    const BaseNode* found_node = find_node(key);

    if (found_node == &nil_) {
      return 0;
    }

    return node_cast(found_node)->key_count;
  }

  std::size_t count_less_than(const Key& key) const {
    std::size_t key_counter = 0;
    const BaseNode* current_node = root_;

    while (current_node != &nil_) {
      const Node* data_node = node_cast(current_node);

      if (data_node->key < key) {
        key_counter += current_node->left->subtree_size;
        key_counter += data_node->key_count;
        current_node = current_node->right;
      } else {
        current_node = current_node->left;
      }
    }

    return key_counter;
  }

  std::size_t size() const { return root_->subtree_size; }

  bool empty() const { return root_ == &nil_; }

  void clear() {
    destroy(root_);
    root_ = &nil_;

    nil_.left = &nil_;
    nil_.right = &nil_;
    nil_.parent = &nil_;
    nil_.color = Color::Black;
    nil_.subtree_size = 0;
  }

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

    BaseNode(BaseNode* nil, Color node_color, std::size_t node_size)
        : left(nil),
          right(nil),
          parent(nil),
          color(node_color),
          subtree_size(node_size) {}

    BaseNode* left;
    BaseNode* right;
    BaseNode* parent;
    Color color;
    std::size_t subtree_size;
  };

  struct Node : BaseNode {
    Node(const Key& key, BaseNode* nil)
        : BaseNode(nil, Color::Red, 1), key(key), key_count(1) {}

    Key key;
    std::size_t key_count;
  };

  BaseNode nil_;
  BaseNode* root_;

  static Node* node_cast(BaseNode* base_node) {
    return static_cast<Node*>(base_node);
  }

  static const Node* node_cast(const BaseNode* base_node) {
    return static_cast<const Node*>(base_node);
  }

  static bool is_red(const BaseNode* base_node) {
    return base_node->color == Color::Red;
  }

  static bool is_black(const BaseNode* base_node) {
    return base_node->color == Color::Black;
  }

  void update_subtree_size(BaseNode* base_node) {
    if (base_node == &nil_) {
      return;
    }

    base_node->subtree_size = base_node->left->subtree_size
                              + base_node->right->subtree_size
                              + node_cast(base_node)->key_count;
  }

  void update_ancestors(BaseNode* base_node) {
    while (base_node != &nil_) {
      update_subtree_size(base_node);
      base_node = base_node->parent;
    }
  }

  void left_rotate(BaseNode* pivot_node) {
    BaseNode* raised_node = pivot_node->right;

    pivot_node->right = raised_node->left;
    if (raised_node->left != &nil_) {
      raised_node->left->parent = pivot_node;
    }

    raised_node->parent = pivot_node->parent;

    if (pivot_node->parent == &nil_) {
      root_ = raised_node;
    } else if (pivot_node == pivot_node->parent->left) {
      pivot_node->parent->left = raised_node;
    } else {
      pivot_node->parent->right = raised_node;
    }

    raised_node->left = pivot_node;
    pivot_node->parent = raised_node;

    update_subtree_size(pivot_node);
    update_subtree_size(raised_node);
  }

  void right_rotate(BaseNode* pivot_node) {
    BaseNode* raised_node = pivot_node->left;

    pivot_node->left = raised_node->right;
    if (raised_node->right != &nil_) {
      raised_node->right->parent = pivot_node;
    }

    raised_node->parent = pivot_node->parent;

    if (pivot_node->parent == &nil_) {
      root_ = raised_node;
    } else if (pivot_node == pivot_node->parent->right) {
      pivot_node->parent->right = raised_node;
    } else {
      pivot_node->parent->left = raised_node;
    }

    raised_node->right = pivot_node;
    pivot_node->parent = raised_node;

    update_subtree_size(pivot_node);
    update_subtree_size(raised_node);
  }

  // Restores red-black invariants after inserting a red node.
  void insert_fixup(BaseNode* node_to_fix) {
    while (is_red(node_to_fix->parent)) {
      BaseNode* parent_node = node_to_fix->parent;
      BaseNode* grandparent_node = parent_node->parent;

      if (parent_node == grandparent_node->left) {
        BaseNode* uncle_node = grandparent_node->right;

        if (is_red(uncle_node)) {
          // Case 1: the parent and uncle are red. Recolor and move upward.
          parent_node->color = Color::Black;
          uncle_node->color = Color::Black;
          grandparent_node->color = Color::Red;

          node_to_fix = grandparent_node;
          continue;
        }

        if (node_to_fix == parent_node->right) {
          // Case 2: convert the inner configuration into an outer one.
          node_to_fix = parent_node;
          left_rotate(node_to_fix);

          // The rotation changed the parent relationships. Refresh pointers.
          parent_node = node_to_fix->parent;
          grandparent_node = parent_node->parent;
        }

        // Case 3: parent and grandparent form an outer configuration.
        parent_node->color = Color::Black;
        grandparent_node->color = Color::Red;
        right_rotate(grandparent_node);
      } else {
        // Mirror image: the parent is the right child of the grandparent.
        BaseNode* uncle_node = grandparent_node->left;

        if (is_red(uncle_node)) {
          // Case 1: the parent and uncle are red. Recolor and move upward.
          parent_node->color = Color::Black;
          uncle_node->color = Color::Black;
          grandparent_node->color = Color::Red;

          node_to_fix = grandparent_node;
          continue;
        }

        if (node_to_fix == parent_node->left) {
          // Case 2: convert the inner configuration into an outer one.
          node_to_fix = parent_node;
          right_rotate(node_to_fix);

          // The rotation changed the parent relationships. Refresh pointers.
          parent_node = node_to_fix->parent;
          grandparent_node = parent_node->parent;
        }

        // Case 3: parent and grandparent form an outer configuration.
        parent_node->color = Color::Black;
        grandparent_node->color = Color::Red;
        left_rotate(grandparent_node);
      }
    }

    root_->color = Color::Black;
  }

  // Replaces old_node in its parent's child link with new_node.
  void transplant(BaseNode* old_node, BaseNode* new_node) {
    if (old_node->parent == &nil_) {
      root_ = new_node;
    } else if (old_node == old_node->parent->left) {
      old_node->parent->left = new_node;
    } else {
      old_node->parent->right = new_node;
    }

    // Delete fix-up needs nil_.parent when new_node is the sentinel.
    new_node->parent = old_node->parent;
  }

  // Returns the node with the smallest key in the subtree.
  BaseNode* find_minimum(BaseNode* subtree_root) const {
    while (subtree_root->left != &nil_) {
      subtree_root = subtree_root->left;
    }

    return subtree_root;
  }

  // Finds a node by key. Returns the sentinel when the key is absent.
  BaseNode* find_node(const Key& key) {
    BaseNode* current_node = root_;

    while (current_node != &nil_) {
      Node* data_node = node_cast(current_node);

      if (key < data_node->key) {
        current_node = current_node->left;
      } else if (data_node->key < key) {
        current_node = current_node->right;
      } else {
        return current_node;
      }
    }

    return &nil_;
  }

  const BaseNode* find_node(const Key& key) const {
    const BaseNode* current_node = root_;

    while (current_node != &nil_) {
      const Node* data_node = node_cast(current_node);

      if (key < data_node->key) {
        current_node = current_node->left;
      } else if (data_node->key < key) {
        current_node = current_node->right;
      } else {
        return current_node;
      }
    }

    return &nil_;
  }

  // Deletes a node that contains exactly one occurrence of its key.
  void erase_node(BaseNode* target_node) {
    BaseNode* moved_node = target_node;
    Color removed_color = moved_node->color;
    BaseNode* fixup_node = &nil_;

    if (target_node->left == &nil_) {
      fixup_node = target_node->right;
      BaseNode* old_parent = target_node->parent;

      transplant(target_node, target_node->right);
      update_ancestors(old_parent);

    } else if (target_node->right == &nil_) {
      fixup_node = target_node->left;
      BaseNode* old_parent = target_node->parent;

      transplant(target_node, target_node->left);
      update_ancestors(old_parent);

    } else {
      // Move the in-order successor into the target node's position.
      moved_node = find_minimum(target_node->right);
      removed_color = moved_node->color;
      fixup_node = moved_node->right;

      if (moved_node->parent == target_node) {
        // Required even if fixup_node is the sentinel.
        fixup_node->parent = moved_node;
      } else {
        BaseNode* old_parent = moved_node->parent;

        transplant(moved_node, moved_node->right);
        update_ancestors(old_parent);

        moved_node->right = target_node->right;
        moved_node->right->parent = moved_node;
      }

      transplant(target_node, moved_node);
      moved_node->left = target_node->left;
      moved_node->left->parent = moved_node;
      moved_node->color = target_node->color;

      update_subtree_size(moved_node);
      update_ancestors(moved_node->parent);
    }

    delete node_cast(target_node);

    if (removed_color == Color::Black) {
      delete_fixup(fixup_node);
    }

    nil_.color = Color::Black;
  }

  // Restores red-black invariants after deleting a black node.
  void delete_fixup(BaseNode* node_to_fix) {
    while (node_to_fix != root_ && is_black(node_to_fix)) {
      BaseNode* parent_node = node_to_fix->parent;

      if (node_to_fix == parent_node->left) {
        BaseNode* sibling_node = parent_node->right;

        if (sibling_node == &nil_) {
          node_to_fix = parent_node;
          continue;
        }

        if (is_red(sibling_node)) {
          // Case 1: sibling is red. Rotate to obtain a black sibling.
          sibling_node->color = Color::Black;
          parent_node->color = Color::Red;
          left_rotate(parent_node);

          sibling_node = node_to_fix->parent->right;
        }

        if (is_black(sibling_node->left) && is_black(sibling_node->right)) {
          // Case 2: sibling and both its children are black.
          sibling_node->color = Color::Red;
          node_to_fix = node_to_fix->parent;
        } else {
          if (is_black(sibling_node->right)) {
            // Case 3: make the far child red.
            sibling_node->left->color = Color::Black;
            sibling_node->color = Color::Red;
            right_rotate(sibling_node);

            sibling_node = node_to_fix->parent->right;
          }

          // Case 4: far child is red; restore balance and finish.
          sibling_node->color = node_to_fix->parent->color;
          node_to_fix->parent->color = Color::Black;
          sibling_node->right->color = Color::Black;
          left_rotate(node_to_fix->parent);

          node_to_fix = root_;
        }
      } else {
        // Mirror image: node_to_fix is the right child of its parent.
        BaseNode* sibling_node = parent_node->left;

        if (sibling_node == &nil_) {
          node_to_fix = parent_node;
          continue;
        }

        if (is_red(sibling_node)) {
          // Case 1: sibling is red. Rotate to obtain a black sibling.
          sibling_node->color = Color::Black;
          parent_node->color = Color::Red;
          right_rotate(parent_node);

          sibling_node = node_to_fix->parent->left;
        }

        if (is_black(sibling_node->right) && is_black(sibling_node->left)) {
          // Case 2: sibling and both its children are black.
          sibling_node->color = Color::Red;
          node_to_fix = node_to_fix->parent;
        } else {
          if (is_black(sibling_node->left)) {
            // Case 3: make the far child red.
            sibling_node->right->color = Color::Black;
            sibling_node->color = Color::Red;
            left_rotate(sibling_node);

            sibling_node = node_to_fix->parent->left;
          }

          // Case 4: far child is red; restore balance and finish.
          sibling_node->color = node_to_fix->parent->color;
          node_to_fix->parent->color = Color::Black;
          sibling_node->left->color = Color::Black;
          right_rotate(node_to_fix->parent);

          node_to_fix = root_;
        }
      }
    }

    node_to_fix->color = Color::Black;
  }

  void destroy(BaseNode* subtree_root) {
    if (subtree_root == &nil_) {
      return;
    }

    std::stack<BaseNode*> node_stack;
    node_stack.push(subtree_root);

    while (!node_stack.empty()) {
      BaseNode* current_node = node_stack.top();
      node_stack.pop();

      if (current_node->left != &nil_) {
        node_stack.push(current_node->left);
      }

      if (current_node->right != &nil_) {
        node_stack.push(current_node->right);
      }

      delete node_cast(current_node);
    }
  }
};
