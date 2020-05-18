#pragma once

#include <vector>
#include <algorithm>
#include <cassert>

template<class T>
class SimpleVector;

template<class T>
using Vector = SimpleVector<T>;

template<class T>
class SimpleVector {
 public:
	typedef T* iterator;
	typedef const T* const_iterator;

	SimpleVector() = default;
	~SimpleVector() {
		Destroy();
	}

	explicit SimpleVector(size_t n) {
		resize(n);
	}

	SimpleVector(const SimpleVector &another) {
		CopyFromRange(another.b_, another.e_);
	}

	template<class InputIterator>
	SimpleVector(
	    InputIterator first,
	    InputIterator last) {
		CopyFromRange(first, last);
	}

	SimpleVector(std::initializer_list<T>&& il) {
		MoveFromRange(il.begin(), il.end());
	}

	SimpleVector &operator=(const SimpleVector &another) {
		if (this == &another) {
			return *this;
		}

		CopyFromRange(another.b_, another.c_);
		return *this;
	}

	SimpleVector &operator=(SimpleVector&& another) {
		if (this == &another) {
			return *this;
		}

		MoveFromRange(another.b_, another.c_);
		return *this;
	}

	T& operator[](size_t id) {
		assert(id < size());
		return b_[id];
	}

	const T& operator[](size_t id) const {
		assert(id < size());
		return b_[id];
	}

	void reserve(size_t n) {
		if (n > size()) {
			ReAllocate(n);
		}
	}

	void resize(size_t n) {
		if (n <= size()) {
			DestroyRange(b_ + n, c_);
			Shrink();
		} else {
			reserve(n);
			for (; c_ != e_; ++c_) {
				Construct(c_);
			}
		}
	}

	void pop_back() {
		assert(!empty());
		--c_;
		c_->~T();
	}

	void push_back(const T &value) {
		emplace_back(value);
	}

	void push_back(T &&value) {
		emplace_back(std::move(value));
	}

	template<class... Args>
	T& emplace_back(Args &&... args) {
		if (c_ == e_) {
			ReAllocate();
		}
		Construct(c_, std::forward<Args>(args)...);
		++c_;
		return back();
	}

	T &front() {
		assert(!empty());
		return *b_;
	}

	const T &front() const {
		assert(!empty());
		return *b_;
	}

	T& back() {
		assert(!empty());
		return *(c_ - 1);
	}

	const T& back() const {
		assert(!empty());
		return *(c_ - 1);
	}

	bool empty() const {
		return b_ == c_;
	}

	size_t size() const {
		return c_ - b_;
	}

	iterator begin() noexcept {
		return b_;
	}

	const_iterator begin() const noexcept {
		return b_;
	}

	iterator end() noexcept {
		return c_;
	}

	const_iterator end() const noexcept {
		return c_;
	}

	iterator erase(iterator pos) {
		return erase(pos, pos + 1);
	}

	iterator erase(iterator first, iterator last) {
		DestroyRange(first, last);
		return first;
	}

 private:
	template<typename... Args>
	static void Construct(T *p, Args &&... args) {
		new (p) T(std::forward<Args>(args)...);
	}

	void ReAllocate() {
		ReAllocate(ComputeNewSize());
	}

	void ReAllocate(size_t new_size) {
		T *new_b = Allocate(new_size);
		T* new_c = new_b;
		for (T *iter = b_; iter != c_; ++iter, ++new_c) {
			Construct(new_c, std::move(*iter));
		}
		Destroy();
		b_ = new_b;
		e_ = new_b + new_size;
		c_ = new_c;
	}

	T* Allocate(size_t n) {
		return (T*)malloc(n * sizeof(T));
	}

	void DeAllocate() {
		free(b_);
	}

	template<class Iterator>
	void CopyFromRange(const Iterator& b, const Iterator& e) {
		DestroyRange(b_, c_);
		for (Iterator iter = b; iter != e; ++iter) {
			emplace_back(*iter);
		}
	}

	template<class Iterator>
	void MoveFromRange(const Iterator &b, const Iterator &e) {
		DestroyRange(b_, c_);
		for (Iterator iter = b; iter != e; ++iter) {
			emplace_back(std::move(*iter));
		}
	}

	size_t ComputeNewSize() {
		return std::max(size_t((e_ - b_) * coefficient_), size() + 1);
	}

	void DestroyRange(T* first, T* last) {
		for (T* iter = first; iter != last; ++iter) {
			(iter)->~T();
		}
		for (T* iter = last; iter != c_; ++iter, ++first) {
			Construct(first, std::move(*iter));
		}
		c_ = first;
	}

	void Destroy() {
		DestroyRange(b_, c_);
		DeAllocate();
	}

	void Shrink() {
		ReAllocate(size());
	}

	T* b_ = nullptr;
	T* e_ = nullptr;
	T* c_ = nullptr;
	static const double coefficient_;
};

template<class T>
double const SimpleVector<T>::coefficient_ = 1.5;