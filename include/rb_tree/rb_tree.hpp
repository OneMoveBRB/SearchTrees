#pragma once

#include <cstddef>
#include <optional>

template <typename Key>
class RBTree {
 public:
  RBTree() : nil_(new BaseNode()), root_(nil_) {}

  ~RBTree() {
    destroy(root_);
    delete nil_;
  }

  RBTree(const RBTree&) = delete;
  RBTree& operator=(const RBTree&) = delete;

  RBTree(RBTree&&) = delete;
  RBTree& operator=(RBTree&&) = delete;

  void insert(const Key& key) {
    BaseNode* parent_node = nil_;
    BaseNode* current_node = root_;

    while (current_node != nil_) {
      parent_node = current_node;
      Node* data_node = as_node(current_node);

      if (key < data_node->key) {
        current_node = current_node->left;
      } else if (data_node->key < key) {
        current_node = current_node->right;
      } else {
        ++data_node->key_cnt;
        update_up(current_node);
        return;
      }
    }

    Node* new_node = new Node(key, nil_);
    new_node->parent = parent_node;

    if (parent_node == nil_) {
      root_ = new_node;
    } else if (key < as_node(parent_node)->key) {
      parent_node->left = new_node;
    } else {
      parent_node->right = new_node;
    }

    update_up(parent_node);
    insert_fixup(new_node);
  }

  bool erase(const Key& key) {
    BaseNode* target_node = find_node(key);

    if (target_node == nil_) {
      return false;
    }

    Node* target_data = as_node(target_node);

    if (target_data->key_cnt > 1) {
      --target_data->key_cnt;
      update_up(target_node);
      return true;
    }

    BaseNode* successor_node = target_node;
    Color original_color = successor_node->color;
    BaseNode* replacement_node = nil_;

    if (target_node->left == nil_) {
      replacement_node = target_node->right;

      BaseNode* old_parent = target_node->parent;

      transplant(target_node, target_node->right);
      update_up(old_parent);
    } else if (target_node->right == nil_) {
      replacement_node = target_node->left;

      BaseNode* old_parent = target_node->parent;

      transplant(target_node, target_node->left);
      update_up(old_parent);
    } else {
      successor_node = minimum(target_node->right);
      original_color = successor_node->color;
      replacement_node = successor_node->right;

      if (successor_node->parent == target_node) {
        replacement_node->parent = successor_node;
      } else {
        BaseNode* old_parent = successor_node->parent;

        transplant(successor_node, successor_node->right);
        update_up(old_parent);

        successor_node->right = target_node->right;
        successor_node->right->parent = successor_node;
      }

      transplant(target_node, successor_node);

      successor_node->left = target_node->left;
      successor_node->left->parent = successor_node;
      successor_node->color = target_node->color;

      recalc(successor_node);
      update_up(successor_node->parent);
    }

    delete as_node(target_node);

    if (original_color == Color::Black) {
      delete_fixup(replacement_node);
    }

    nil_->color = Color::Black;

    return true;
  }

  std::optional<Key> lower_bound(const Key& key) const {
    BaseNode* current_node = root_;
    BaseNode* answer_node = nil_;

    while (current_node != nil_) {
      Node* data_node = as_node(current_node);

      if (!(data_node->key < key)) {
        answer_node = current_node;
        current_node = current_node->left;
      } else {
        current_node = current_node->right;
      }
    }

    if (answer_node == nil_) {
      return std::nullopt;
    }

    return as_node(answer_node)->key;
  }

  std::optional<Key> kth(std::size_t order) const {
    if (order == 0 || order > size()) {
      return std::nullopt;
    }

    BaseNode* current_node = root_;

    while (current_node != nil_) {
      const std::size_t left_size = current_node->left->subtree_size;

      const std::size_t key_count = as_node(current_node)->key_cnt;

      if (order <= left_size) {
        current_node = current_node->left;
      } else if (order <= left_size + key_count) {
        return as_node(current_node)->key;
      } else {
        order -= left_size + key_count;
        current_node = current_node->right;
      }
    }

    return std::nullopt;
  }

  std::size_t count(const Key& key) const {
    BaseNode* found_node = find_node(key);

    if (found_node == nil_) {
      return 0;
    }

    return as_node(found_node)->key_cnt;
  }

  std::size_t count_less_than(const Key& key) const {
    std::size_t less_count = 0;
    BaseNode* current_node = root_;

    while (current_node != nil_) {
      Node* data_node = as_node(current_node);

      if (data_node->key < key) {
        less_count += current_node->left->subtree_size;
        less_count += data_node->key_cnt;

        current_node = current_node->right;
      } else {
        current_node = current_node->left;
      }
    }

    return less_count;
  }

  std::size_t size() const { return root_->subtree_size; }

  bool empty() const { return root_ == nil_; }

  void clear() {
    destroy(root_);
    root_ = nil_;

    nil_->left = nil_;
    nil_->right = nil_;
    nil_->parent = nil_;
    nil_->color = Color::Black;
    nil_->subtree_size = 0;
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

  static Node* as_node(BaseNode* base_node) {
    return static_cast<Node*>(base_node);
  }

  static bool is_red(const BaseNode* base_node) {
    return base_node->color == Color::Red;
  }

  static bool is_black(const BaseNode* base_node) {
    return base_node->color == Color::Black;
  }

  void recalc(BaseNode* base_node) {
    if (base_node == nil_) {
      return;
    }

    base_node->subtree_size = base_node->left->subtree_size
                              + base_node->right->subtree_size
                              + as_node(base_node)->key_cnt;
  }

  void update_up(BaseNode* base_node) {
    while (base_node != nil_) {
      recalc(base_node);
      base_node = base_node->parent;
    }
  }

  void left_rotate(BaseNode* pivot_node) {
    BaseNode* raised_node = pivot_node->right;

    pivot_node->right = raised_node->left;

    if (raised_node->left != nil_) {
      raised_node->left->parent = pivot_node;
    }

    raised_node->parent = pivot_node->parent;

    if (pivot_node->parent == nil_) {
      root_ = raised_node;
    } else if (pivot_node == pivot_node->parent->left) {
      pivot_node->parent->left = raised_node;
    } else {
      pivot_node->parent->right = raised_node;
    }

    raised_node->left = pivot_node;
    pivot_node->parent = raised_node;

    recalc(pivot_node);
    recalc(raised_node);
  }

  void right_rotate(BaseNode* pivot_node) {
    BaseNode* raised_node = pivot_node->left;

    pivot_node->left = raised_node->right;

    if (raised_node->right != nil_) {
      raised_node->right->parent = pivot_node;
    }

    raised_node->parent = pivot_node->parent;

    if (pivot_node->parent == nil_) {
      root_ = raised_node;
    } else if (pivot_node == pivot_node->parent->right) {
      pivot_node->parent->right = raised_node;
    } else {
      pivot_node->parent->left = raised_node;
    }

    raised_node->right = pivot_node;
    pivot_node->parent = raised_node;

    recalc(pivot_node);
    recalc(raised_node);
  }

  void insert_fixup(BaseNode* node_to_fix) {
    while (is_red(node_to_fix->parent)) {
      if (node_to_fix->parent == node_to_fix->parent->parent->left) {
        BaseNode* uncle_node = node_to_fix->parent->parent->right;

        if (is_red(uncle_node)) {
          node_to_fix->parent->color = Color::Black;
          uncle_node->color = Color::Black;
          node_to_fix->parent->parent->color = Color::Red;

          node_to_fix = node_to_fix->parent->parent;
        } else {
          if (node_to_fix == node_to_fix->parent->right) {
            node_to_fix = node_to_fix->parent;
            left_rotate(node_to_fix);
          }

          node_to_fix->parent->color = Color::Black;
          node_to_fix->parent->parent->color = Color::Red;

          right_rotate(node_to_fix->parent->parent);
        }
      } else {
        BaseNode* uncle_node = node_to_fix->parent->parent->left;

        if (is_red(uncle_node)) {
          node_to_fix->parent->color = Color::Black;
          uncle_node->color = Color::Black;
          node_to_fix->parent->parent->color = Color::Red;

          node_to_fix = node_to_fix->parent->parent;
        } else {
          if (node_to_fix == node_to_fix->parent->left) {
            node_to_fix = node_to_fix->parent;
            right_rotate(node_to_fix);
          }

          node_to_fix->parent->color = Color::Black;
          node_to_fix->parent->parent->color = Color::Red;

          left_rotate(node_to_fix->parent->parent);
        }
      }
    }

    root_->color = Color::Black;
  }

  void transplant(BaseNode* old_node, BaseNode* new_node) {
    if (old_node->parent == nil_) {
      root_ = new_node;
    } else if (old_node == old_node->parent->left) {
      old_node->parent->left = new_node;
    } else {
      old_node->parent->right = new_node;
    }

    new_node->parent = old_node->parent;
  }

  BaseNode* minimum(BaseNode* subtree_root) const {
    while (subtree_root->left != nil_) {
      subtree_root = subtree_root->left;
    }

    return subtree_root;
  }

  BaseNode* find_node(const Key& key) const {
    BaseNode* current_node = root_;

    while (current_node != nil_) {
      Node* data_node = as_node(current_node);

      if (key < data_node->key) {
        current_node = current_node->left;
      } else if (data_node->key < key) {
        current_node = current_node->right;
      } else {
        return current_node;
      }
    }

    return nil_;
  }

  void delete_fixup(BaseNode* node_to_fix) {
    while (node_to_fix != root_ && is_black(node_to_fix)) {
      if (node_to_fix == node_to_fix->parent->left) {
        BaseNode* sibling_node = node_to_fix->parent->right;

        if (sibling_node == nil_) {
          node_to_fix = node_to_fix->parent;
          continue;
        }

        if (is_red(sibling_node)) {
          sibling_node->color = Color::Black;
          node_to_fix->parent->color = Color::Red;

          left_rotate(node_to_fix->parent);

          sibling_node = node_to_fix->parent->right;
        }

        if (is_black(sibling_node->left) && is_black(sibling_node->right)) {
          sibling_node->color = Color::Red;
          node_to_fix = node_to_fix->parent;
        } else {
          if (is_black(sibling_node->right)) {
            sibling_node->left->color = Color::Black;
            sibling_node->color = Color::Red;

            right_rotate(sibling_node);

            sibling_node = node_to_fix->parent->right;
          }

          sibling_node->color = node_to_fix->parent->color;
          node_to_fix->parent->color = Color::Black;
          sibling_node->right->color = Color::Black;

          left_rotate(node_to_fix->parent);

          node_to_fix = root_;
        }
      } else {
        BaseNode* sibling_node = node_to_fix->parent->left;

        if (sibling_node == nil_) {
          node_to_fix = node_to_fix->parent;
          continue;
        }

        if (is_red(sibling_node)) {
          sibling_node->color = Color::Black;
          node_to_fix->parent->color = Color::Red;

          right_rotate(node_to_fix->parent);

          sibling_node = node_to_fix->parent->left;
        }

        if (is_black(sibling_node->right) && is_black(sibling_node->left)) {
          sibling_node->color = Color::Red;
          node_to_fix = node_to_fix->parent;
        } else {
          if (is_black(sibling_node->left)) {
            sibling_node->right->color = Color::Black;
            sibling_node->color = Color::Red;

            left_rotate(sibling_node);

            sibling_node = node_to_fix->parent->left;
          }

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

  void destroy(BaseNode* base_node) {
    if (base_node == nil_) {
      return;
    }

    destroy(base_node->left);
    destroy(base_node->right);

    delete as_node(base_node);
  }

  BaseNode* nil_;
  BaseNode* root_;
};
