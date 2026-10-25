#pragma once

#include <cstddef>
#include <optional>
#include <stack>

#ifndef NDEBUG
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_set>
#endif // !NDEBUG

namespace search_trees {

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
    BaseNode* found_node =
        descend_from_root(InsertVisitor{*this, parent_node, key});

    if (found_node != &nil_) {
      return;
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
    BaseNode* target_node = descend_from_root(FindVisitor{key});

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
    const BaseNode* result_node = &nil_;
    descend_from_root(LowerBoundVisitor{key, result_node});

    if (result_node == &nil_) {
      return std::nullopt;
    }

    return node_cast(result_node)->key;
  }

  std::optional<Key> kth_smallest(std::size_t index) const {
    if (index == 0 || index > size()) {
      return std::nullopt;
    }

    const BaseNode* result_node = descend_from_root(KthSmallestVisitor{index});

    if (result_node == &nil_) {
      return std::nullopt;
    }

    return node_cast(result_node)->key;
  }

  std::size_t count(const Key& key) const {
    const BaseNode* found_node = descend_from_root(FindVisitor{key});

    if (found_node == &nil_) {
      return 0;
    }

    return node_cast(found_node)->key_count;
  }

  std::size_t count_less_than(const Key& key) const {
    std::size_t key_counter = 0;
    descend_from_root(CountLessVisitor{key, key_counter});
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

#ifndef NDEBUG
  // Debug-only API. Definitions and implementation details are kept private.
  bool debug_dump(const char* filename) const {
    return debug_dump_impl(filename);
  }

  bool verify_invariants(std::string* error_message = nullptr) const {
    return verify_invariants_impl(error_message);
  }
#endif // !NDEBUG

 private:
  enum class Color {
    Red = 0,
    Black = 1
  };

  enum class TraversalDirection {
    Left = 0,
    Right = 1,
    Stop = 2
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
    Node(const Key& node_key, BaseNode* nil)
        : BaseNode(nil, Color::Red, 1), key(node_key), key_count(1) {}

    Key key;
    std::size_t key_count;
  };

  struct FindVisitor {
    const Key& key;

    TraversalDirection operator()(const Node* node) const {
      if (key < node->key) {
        return TraversalDirection::Left;
      }

      if (node->key < key) {
        return TraversalDirection::Right;
      }

      return TraversalDirection::Stop;
    }
  };

  struct InsertVisitor {
    RBTree& tree;
    BaseNode*& parent;
    const Key& key;

    TraversalDirection operator()(Node* node) const {
      parent = node;

      if (key < node->key) {
        return TraversalDirection::Left;
      }

      if (node->key < key) {
        return TraversalDirection::Right;
      }

      ++node->key_count;
      tree.update_ancestors(node);

      return TraversalDirection::Stop;
    }
  };

  struct LowerBoundVisitor {
    const Key& key;
    const BaseNode*& result_node;

    TraversalDirection operator()(const Node* node) const {
      if (!(node->key < key)) {
        result_node = node;
        return TraversalDirection::Left;
      }

      return TraversalDirection::Right;
    }
  };

  struct KthSmallestVisitor {
    std::size_t& index;

    TraversalDirection operator()(const Node* node) const {
      const std::size_t left_size = node->left->subtree_size;

      if (index <= left_size) {
        return TraversalDirection::Left;
      }

      if (index <= left_size + node->key_count) {
        return TraversalDirection::Stop;
      }

      index -= left_size + node->key_count;
      return TraversalDirection::Right;
    }
  };

  struct CountLessVisitor {
    const Key& key;
    std::size_t& key_counter;

    TraversalDirection operator()(const Node* node) const {
      if (node->key < key) {
        key_counter += node->left->subtree_size;
        key_counter += node->key_count;
        return TraversalDirection::Right;
      }

      return TraversalDirection::Left;
    }
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

  template <typename Visitor>
  BaseNode* descend_from_root(Visitor visit) {
    BaseNode* current_node = root_;

    while (current_node != &nil_) {
      TraversalDirection direction = visit(node_cast(current_node));
      if (direction == TraversalDirection::Stop) {
        return current_node;
      }

      current_node = (direction == TraversalDirection::Left) ?
                         current_node->left :
                         current_node->right;
    }

    return &nil_;
  }

  template <typename Visitor>
  const BaseNode* descend_from_root(Visitor visit) const {
    const BaseNode* nil_node = &nil_;
    const BaseNode* current_node = root_;

    while (current_node != nil_node) {
      const TraversalDirection direction = visit(node_cast(current_node));
      if (direction == TraversalDirection::Stop) {
        return current_node;
      }

      current_node = (direction == TraversalDirection::Left) ?
                         current_node->left :
                         current_node->right;
    }

    return nil_node;
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

  // debug ---------------------------------------------------------------------

#ifndef NDEBUG
  struct VerificationResult {
    bool valid;
    std::size_t computed_size;
    std::size_t black_height;
  };

  bool debug_dump_impl(const char* filename) const {
    if (filename == nullptr) {
      return false;
    }

    std::ofstream output_file(filename);
    if (!output_file.is_open()) {
      return false;
    }

    output_file << "digraph Tree {\n";
    output_file << "\trankdir=TB;\n";
    output_file << "\tnode [shape=record, style=filled];\n";
    output_file << "\tedge [fontsize=10, color=black];\n\n";

    if (root_ != &nil_) {
      std::stack<const BaseNode*> pending_nodes;
      std::unordered_set<const BaseNode*> emitted_nodes;
      pending_nodes.push(root_);

      while (!pending_nodes.empty()) {
        const BaseNode* current_node = pending_nodes.top();
        pending_nodes.pop();

        if (current_node == &nil_
            || !emitted_nodes.insert(current_node).second) {
          continue;
        }

        const Node* data_node = node_cast(current_node);
        const std::string current_address = pointer_to_string(current_node);
        const std::string current_identifier =
            dot_node_identifier(current_node);

        std::ostringstream key_stream;
        key_stream << data_node->key;
        const std::string key_text = key_stream.str();

        const bool red_node = current_node->color == Color::Red;
        const char* fill_color = red_node ? "#F28B82" : "#303030";
        const char* font_color = red_node ? "black" : "white";
        const char* color_text = red_node ? "RED" : "BLACK";

        output_file << "\t" << current_identifier << " [label=\"{{{<f0> ";
        write_dot_escaped(output_file, current_address);
        output_file << " | <f1> type = RB_NODE | <f2> key = ";
        write_dot_escaped(output_file, key_text);
        output_file << " | <f3> color = " << color_text
                    << " | <f4> key_count = " << data_node->key_count
                    << " | <f5> subtree_size = " << current_node->subtree_size
                    << "}} | {<f6> left: ";

        if (current_node->left == &nil_) {
          output_file << "(nil)";
        } else {
          write_dot_escaped(output_file, pointer_to_string(current_node->left));
        }

        output_file << " | <f7> right: ";

        if (current_node->right == &nil_) {
          output_file << "(nil)";
        } else {
          write_dot_escaped(output_file,
                            pointer_to_string(current_node->right));
        }

        output_file << "}}\", fillcolor=\"" << fill_color << "\", fontcolor=\""
                    << font_color << "\"];\n";

        if (current_node->left != &nil_) {
          write_dot_edge(output_file, current_node, current_node->left, "f6",
                         "red");
          pending_nodes.push(current_node->left);
        }

        if (current_node->right != &nil_) {
          write_dot_edge(output_file, current_node, current_node->right, "f7",
                         "green");
          pending_nodes.push(current_node->right);
        }
      }
    }

    output_file << "}\n";
    return output_file.good();
  }

  bool verify_invariants_impl(std::string* error_message) const {
    if (error_message != nullptr) {
      error_message->clear();
    }

    if (nil_.left != &nil_ || nil_.right != &nil_) {
      return set_verification_error(
          error_message, "NIL left/right pointers must refer to NIL");
    }
    if (nil_.color != Color::Black) {
      return set_verification_error(error_message, "NIL must be black");
    }
    if (nil_.subtree_size != 0) {
      return set_verification_error(error_message,
                                    "NIL subtree_size must be zero");
    }
    if (root_ == nullptr) {
      return set_verification_error(error_message, "root pointer is null");
    }
    if (root_ == &nil_) {
      return true;
    }
    if (root_->parent != &nil_) {
      return set_verification_error(error_message, "root parent must be NIL");
    }
    if (root_->color != Color::Black) {
      return set_verification_error(error_message, "root must be black");
    }

    std::unordered_set<const BaseNode*> visited_nodes;
    const VerificationResult result = verify_subtree(
        root_, &nil_, nullptr, nullptr, visited_nodes, error_message);
    return result.valid;
  }

  static bool set_verification_error(std::string* error_message,
                                     const char* message) {
    if (error_message != nullptr) {
      *error_message = message;
    }
    return false;
  }

  VerificationResult verification_failure(std::string* error_message,
                                          const char* message) const {
    set_verification_error(error_message, message);
    return {false, 0, 0};
  }

  VerificationResult verify_subtree(
      const BaseNode* subtree_root, const BaseNode* expected_parent,
      const Key* lower_bound, const Key* upper_bound,
      std::unordered_set<const BaseNode*>& visited_nodes,
      std::string* error_message) const {
    if (subtree_root == nullptr) {
      return verification_failure(error_message,
                                  "a child pointer is null instead of NIL");
    }

    // NIL contributes no elements and one black node to black height.
    if (subtree_root == &nil_) {
      return {true, 0, 1};
    }

    if (!visited_nodes.insert(subtree_root).second) {
      return verification_failure(
          error_message,
          "a node is reachable more than once (cycle/shared child)");
    }

    if (subtree_root->parent != expected_parent) {
      return verification_failure(error_message,
                                  "a node has an incorrect parent pointer");
    }

    if (subtree_root->color != Color::Red
        && subtree_root->color != Color::Black) {
      return verification_failure(error_message, "a node has an invalid color");
    }

    const Node* current_data = node_cast(subtree_root);
    if (current_data->key_count == 0) {
      return verification_failure(error_message,
                                  "key_count must be greater than zero");
    }

    if (lower_bound != nullptr && !(*lower_bound < current_data->key)) {
      return verification_failure(error_message,
                                  "BST lower-bound ordering is violated");
    }
    if (upper_bound != nullptr && !(current_data->key < *upper_bound)) {
      return verification_failure(error_message,
                                  "BST upper-bound ordering is violated");
    }

    if (is_red(subtree_root)
        && (is_red(subtree_root->left) || is_red(subtree_root->right))) {
      return verification_failure(error_message, "a red node has a red child");
    }

    const VerificationResult left_result =
        verify_subtree(subtree_root->left, subtree_root, lower_bound,
                       &current_data->key, visited_nodes, error_message);
    if (!left_result.valid) {
      return left_result;
    }

    const VerificationResult right_result =
        verify_subtree(subtree_root->right, subtree_root, &current_data->key,
                       upper_bound, visited_nodes, error_message);
    if (!right_result.valid) {
      return right_result;
    }

    if (left_result.black_height != right_result.black_height) {
      return verification_failure(error_message,
                                  "left and right black heights differ");
    }

    const std::size_t calculated_size = left_result.computed_size
                                        + right_result.computed_size
                                        + current_data->key_count;
    if (subtree_root->subtree_size != calculated_size) {
      return verification_failure(
          error_message,
          "subtree_size does not match child sizes and key_count");
    }

    const std::size_t calculated_black_height =
        left_result.black_height
        + (subtree_root->color == Color::Black ? 1 : 0);

    return {true, calculated_size, calculated_black_height};
  }

  static std::string pointer_to_string(const BaseNode* node) {
    std::ostringstream address_stream;
    address_stream << static_cast<const void*>(node);
    return address_stream.str();
  }

  static std::string dot_node_identifier(const BaseNode* node) {
    return "node_" + pointer_to_string(node);
  }

  static void write_dot_escaped(std::ostream& output, const std::string& text) {
    for (char character : text) {
      switch (character) {
        case '\\':
          output << "\\\\";
          break;
        case '"':
          output << "\\\"";
          break;
        case '|':
          output << "\\|";
          break;
        case '{':
          output << "\\{";
          break;
        case '}':
          output << "\\}";
          break;
        case '<':
          output << "\\<";
          break;
        case '>':
          output << "\\>";
          break;
        case '\n':
          output << "\\n";
          break;
        case '\r':
          output << "\\r";
          break;
        default:
          output << character;
          break;
      }
    }
  }

  static void write_dot_edge(std::ostream& output, const BaseNode* parent_node,
                             const BaseNode* child_node, const char* port_name,
                             const char* edge_color) {
    const std::string parent_id = dot_node_identifier(parent_node);
    const std::string child_id = dot_node_identifier(child_node);

    output << "\t" << parent_id << ':' << port_name << " -> " << child_id
           << " [color=" << edge_color << ", arrowhead=normal];\n";
    output << "\t" << child_id << " -> " << parent_id << ':' << port_name
           << " [color=" << edge_color << ", arrowhead=normal];\n";
  }
#endif  // !NDEBUG
};

}  // namespace search_trees
