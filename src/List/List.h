#pragma once

#include <memory>
#include <cassert>

template<typename T>
class List {
 public:
	struct ListNode {
		using Ptr = std::unique_ptr<ListNode>;

		explicit ListNode(T &&data) : data_(std::move(data)) {}
		explicit ListNode(const T &data) : data_(data) {}

		void LinkBefore(Ptr &&next) {
			next_ = std::move(next);
		}

		bool IsLinked() const {
			return next_ != nullptr;
		}

		T &GetItem() {
			return data_;
		}

		const T &GetItem() const {
			return data_;
		}

	 private:
		friend List;

		Ptr next_ = nullptr;
		T data_;
	};

	void PushFront(T&& data) {
		auto new_head = std::make_unique<ListNode>(std::move(data));
		new_head->LinkBefore(std::move(head_));
		head_ = std::move(new_head);
	}

	void PushFront(const T &data) {
		auto new_head = std::make_unique<ListNode>(data);
		new_head->LinkBefore(std::move(head_));
		head_ = std::move(new_head);
	}

	T PopFront() {
		assert(!IsEmpty());
		T result = std::move(head_->GetItem());
		head_ = std::move(head_->next_);
		return std::move(result);
	}

	bool IsEmpty() const noexcept {
		return head_ == nullptr;
	}

	List() = default;

	List(const List& another) {
		ListNode* current = nullptr;
		for (const auto& item: another) {
			if (head_ == nullptr) {
				head_ = std::make_unique<ListNode>(item.GetItem());
				current = head_.get();
			} else {
				current->next_ = std::make_unique<ListNode>(item.GetItem());
				current = current->next_.get();
			}
		}
	}

	size_t Size() const {
		return std::distance(begin(), end());
	}

	void Clear() {
		head_ = nullptr;
	}

	template<class U>
	class IteratorImpl : public std::iterator<std::input_iterator_tag, U> {
	 public:
		IteratorImpl() : current_(nullptr) {
		}

		IteratorImpl &operator++() {
			current_ = current_->next_.get();
			return *this;
		}

		bool operator == (const IteratorImpl &another) const {
			return current_ == another.current_;
		}

		bool operator != (const IteratorImpl &another) const {
			return !(*this == another);
		}

		typename IteratorImpl::reference operator*() const {
			return *current_;
		}

		typename IteratorImpl::pointer operator->() const {
			return current_;
		}

	 private:
		friend List;

		explicit IteratorImpl(typename IteratorImpl::pointer begin) : current_(begin) {
		}

		typename IteratorImpl::pointer current_;
	};

	using Iterator = IteratorImpl<ListNode>;
	using ConstIterator = IteratorImpl<const ListNode>;

	Iterator begin() {
		return Iterator(head_.get());
	}

	Iterator end() {
		return Iterator(nullptr);
	}

	ConstIterator begin() const {
		return ConstIterator(head_.get());
	}

	ConstIterator end() const {
		return ConstIterator(nullptr);
	}

 private:
	typename ListNode::Ptr head_ = nullptr;
};
