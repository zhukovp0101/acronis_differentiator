#pragma once

#include <functional>
#include <memory>

#include "../Vector/Vector.h"

template<class T>
class Tree {
 public:
	struct Node {
	 public:
		using Ptr = std::shared_ptr<Node>;
		static void Attach(Ptr parent, Ptr child) {
			child->parent_ = parent;
			parent->children_.push_back(std::move(child));
		}
		explicit Node(T value) : value_(std::move(value)){};
		Node() = default;
		std::weak_ptr<Node> parent_;
		Vector<Ptr> children_;
		T value_;
	};

	class PostOrderIterator {
	 public:
		using value_type = Node;
		using pointer = typename Node::Ptr&;
		using const_pointer = const typename Node::Ptr&;
		using reference = value_type &;
		using const_reference = const value_type &;
		using difference_type = ptrdiff_t;
		using iterator_category = std::input_iterator_tag;

	 public:
		PostOrderIterator &operator++() {
			if (IsEnd()) {
				return *this;
			}

			if (owner_->IsRoot(node_)) {
				node_ = nullptr;
				return *this;
			}

			size_t position = location_.back();
			location_.pop_back();
			auto parent = node_->parent_.lock();
			if (position + 1 < parent->children_.size()) {
				node_ = parent->children_[position + 1];
				location_.push_back(position + 1);
				while (!node_->children_.empty()) {
					node_ = node_->children_[0];
					location_.push_back(0);
				}
			} else {
				node_ = parent;
			}

			return *this;
		}

		bool operator==(const PostOrderIterator &another) const {
			return this->node_ == another.node_;
		}

		bool operator!=(const PostOrderIterator &another) const {
			return !(*this == another);
		}

		pointer operator->() {
			return node_;
		}

		reference operator*() {
			return *node_;
		}

		const_pointer operator->() const {
			return node_;
		}

		const_reference operator*() const {
			return *node_;
		}

//		pointer Ptr() {
//			return node_;
//		}

	 private:
		friend Tree<T>;
		explicit PostOrderIterator(const Tree<T>* owner, typename Node::Ptr node) : owner_(owner), node_(std::move(node)) {
			GetLocation(location_, node_);
		}

		void GetLocation(Vector<size_t> &location, typename Node::Ptr node) {
			if (owner_->IsRoot(node)) {
				return;
			}

			auto parent = node->parent_.lock();

			GetLocation(location_, parent);

			location_.push_back(owner_->GetId(node));
		}

		bool IsEnd() const {
			return node_ == nullptr;
		}

		typename Node::Ptr node_;
		Vector<size_t> location_;
		const Tree<T>* owner_;
	};

	Tree(): root_(nullptr) {}
	explicit Tree(typename Node::Ptr root) : root_(std::move(root)) {}

	template<class D>
	static Tree<T> CreateLike(const Tree<D>& model, std::function<void(const typename Tree<D>::PostOrderIterator&, typename Tree<T>::PostOrderIterator&)> func) {
		auto new_tree = Tree<T>(CreateLikeNode<D>(model.GetRoot()));

		auto new_tree_iter = new_tree.begin();
		for (auto&& model_iter = model.begin(); model_iter != model.end(); ++model_iter) {
			func(model_iter, new_tree_iter);
			++new_tree_iter;
		}

		return new_tree;
	}

	Tree<T> Replace(typename Node::Ptr old_node, typename Node::Ptr new_node) {
		if (!IsRoot(old_node)) {
			auto parent = old_node->parent_.lock();
			new_node->parent_ = parent;
			parent->children_[GetId(old_node)] = std::move(new_node);
			old_node->parent_ = std::weak_ptr<Node>();
		} else {
			root_ = new_node;
		}
		return Tree<T>(std::move(old_node));
	}

	Tree<T> ExtractSubTree(typename Node::Ptr node) {
		if (!IsRoot(node)) {
			auto parent = node->parent_.lock();
			parent->children_.erase(parent->children_.begin() + GetId(node));
			node->parent_ = std::weak_ptr<Node>();
		} else {
			root_ = nullptr;
		}
		return Tree<T>(std::move(node));
	}

	Tree<T> Replace(PostOrderIterator& old_iter, typename Node::Ptr new_node) {
		auto old_node = old_iter.node_;
		old_iter.node_ = new_node;
		return Replace(std::move(old_node), std::move(new_node));
	}

	Tree<T> ExtractSubTree(PostOrderIterator& iter) {
		auto old_node = iter.node_;
		++iter;
		return ExtractSubTree(std::move(old_node));
	}

	typename Node::Ptr &GetRoot() {
		return root_;
	}

	const typename Node::Ptr &GetRoot() const {
		return root_;
	}

	bool IsRoot(const typename Tree<T>::Node::Ptr &node) const {
		return node == root_;
	}

	PostOrderIterator begin() const {
		auto node = root_;
		while (!node->children_.empty()) {
			node = node->children_[0];
		}
		return PostOrderIterator(this, node);
	}

	PostOrderIterator end() const {
		return ++PostOrderIterator(this, root_);
	}

 private:
	template<class D>
	static typename Tree<T>::Node::Ptr CreateLikeNode(const typename Tree<D>::Node::Ptr &model) {
		if (model == nullptr) {
			return nullptr;
		}

		auto node = std::make_shared<Tree<T>::Node>();
		for (const typename Tree<D>::Node::Ptr &child : model->children_) {
			Tree<T>::Node::Attach(node, CreateLikeNode<D>(child));
		}

		return node;
	}

	int GetId (const typename Node::Ptr& node) const {
		auto parent = node->parent_.lock();
		for (size_t i = 0; i < parent->children_.size(); ++i) {
			if (parent->children_[i] == node) {
				return i;
			}
		}

		return -1;
	}

	typename Node::Ptr root_;
};