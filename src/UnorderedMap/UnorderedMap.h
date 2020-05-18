#pragma once

#include <functional>
#include <unordered_map>

#include <List.h>
#include <Vector.h>

template <class Key, class Value, class Hash = std::hash<Key>>
class SimpleUnorderedMap;

template <class Key, class Value>
using UnorderedMap = SimpleUnorderedMap<Key, Value>;

template <class Key, class Value, class Hash>
class SimpleUnorderedMap {
 public:
  // todo: делать рехеш при маленьком лоад факторе, чтобы итерирование работало
  // за правильную асимптотику;
  SimpleUnorderedMap() = default;

  SimpleUnorderedMap(std::initializer_list<std::pair<Key, Value>> &&il) {
    for (auto &&item : il) {
      insert(std::move(item));
    }
  }

  bool empty() {
    for (const auto &list : data_) {
      if (!list.IsEmpty()) {
        return false;
      }
    }

    return true;
  }

  template <class U, class VectorIter, class ListIter>
  class IteratorImpl : public std::iterator<std::input_iterator_tag, U> {
   public:
    IteratorImpl &operator++() {
      ++list_iter_;
      while (list_iter_ == title_iter_->end()) {
        ++title_iter_;
        if (title_iter_ == end_iter_) {
          list_iter_ = ListIter();
          return *this;
        }
        list_iter_ = title_iter_->begin();
      }
      return *this;
    }

    bool operator==(const IteratorImpl &another) const {
      return (title_iter_ == another.title_iter_) &&
             (list_iter_ == another.list_iter_);
    }

    bool operator!=(const IteratorImpl &another) const {
      return !(*this == another);
    }

    typename IteratorImpl::reference operator*() const {
      return list_iter_->GetItem();
    }

    typename IteratorImpl::pointer operator->() const {
      return &(list_iter_->GetItem());
    }

   private:
    friend SimpleUnorderedMap;
    IteratorImpl(VectorIter title_iter, VectorIter end_iter, ListIter list_iter)
        : title_iter_(std::move(title_iter)),
          end_iter_(std::move(end_iter)),
          list_iter_(std::move(list_iter)) {}

    VectorIter title_iter_;
    VectorIter end_iter_;
    ListIter list_iter_;
  };

  using Iterator =
      IteratorImpl<std::pair<const Key, Value>,
                   typename Vector<List<std::pair<const Key, Value>>>::iterator,
                   typename List<std::pair<const Key, Value>>::Iterator>;
  using ConstIterator = IteratorImpl<
      const std::pair<const Key, Value>,
      typename Vector<List<std::pair<const Key, Value>>>::const_iterator,
      typename List<std::pair<const Key, Value>>::ConstIterator>;

  std::pair<Iterator, bool> insert(std::pair<const Key, Value> &&new_item) {
    CheckReHash();

    auto [title_iter, item_iter] = FindTitleItem(new_item.first);
    if (item_iter != title_iter->end()) {
      return std::pair{Iterator(title_iter, data_.end(), item_iter), false};
    }

    title_iter->PushFront(std::move(new_item));
    ++size_;
    return std::pair{Iterator(title_iter, data_.end(), title_iter->begin()),
                     true};
  }

  std::pair<Iterator, bool> insert(
      const std::pair<const Key, Value> &new_item) {
    CheckReHash();

    auto [title_iter, item_iter] = FindTitleItem(new_item.first);
    if (item_iter != title_iter->end()) {
      return std::pair{Iterator(title_iter, data_.end(), item_iter), false};
    }

    title_iter->PushFront(new_item);
    ++size_;
    return std::pair{Iterator(title_iter, data_.end(), title_iter->begin()),
                     true};
  }

  ConstIterator end() const {
    return ConstIterator(
        data_.end(), data_.end(),
        typename List<std::pair<const Key, Value>>::ConstIterator());
  }

  Iterator end() {
    return Iterator(data_.end(), data_.end(),
                    typename List<std::pair<const Key, Value>>::Iterator());
  }

  ConstIterator begin() const {
    return ConstIterator(data_.begin(), data_.end(), data_.begin()->begin());
  }

  Iterator begin() {
    return Iterator(data_.begin(), data_.end(), data_.begin()->begin());
  }

  Iterator find(const Key &item) {
    auto [title_iter, item_iter] = FindTitleItem(item);
    if (item_iter != title_iter->end()) {
      return Iterator(title_iter, data_.end(), item_iter);
    }

    return Iterator(data_.end(), data_.end(),
                    typename List<std::pair<const Key, Value>>::Iterator());
  }

  ConstIterator find(const Key &item) const {
    auto [title_iter, item_iter] = FindTitleItem(item);
    if (item_iter != title_iter->end()) {
      return ConstIterator(title_iter, data_.end(), item_iter);
    }

    return ConstIterator(
        data_.end(), data_.end(),
        typename List<std::pair<const Key, Value>>::ConstIterator());
  }

  size_t size() const { return size_; }

 private:
  typename List<std::pair<const Key, Value>>::Iterator FindItem(
      typename Vector<List<std::pair<const Key, Value>>>::iterator title_iter,
      const Key &item) {
    for (auto list_iter = title_iter->begin(); list_iter != title_iter->end();
         ++list_iter) {
      if (list_iter->GetItem().first == item) {
        return list_iter;
      }
    }

    return title_iter->end();
  }

  typename List<std::pair<const Key, Value>>::ConstIterator FindItem(
      typename Vector<List<std::pair<const Key, Value>>>::const_iterator
          title_iter,
      const Key &item) const {
    for (auto list_iter = title_iter->begin(); list_iter != title_iter->end();
         ++list_iter) {
      if (list_iter->GetItem().first == item) {
        return list_iter;
      }
    }

    return title_iter->end();
  }

  typename Vector<List<std::pair<const Key, Value>>>::iterator FindTitle(
      const Key &item) {
    return hash_(item) % data_.size() + data_.begin();
  }

  typename Vector<List<std::pair<const Key, Value>>>::const_iterator FindTitle(
      const Key &item) const {
    return hash_(item) % data_.size() + data_.begin();
  }

  std::pair<typename Vector<List<std::pair<const Key, Value>>>::const_iterator,
            typename List<std::pair<const Key, Value>>::ConstIterator>
  FindTitleItem(const Key &item) const {
    auto title_iter = FindTitle(item);
    auto item_iter = FindItem(title_iter, item);
    return std::pair{title_iter, item_iter};
  }

  std::pair<typename Vector<List<std::pair<const Key, Value>>>::iterator,
            typename List<std::pair<const Key, Value>>::Iterator>
  FindTitleItem(const Key &item) {
    auto title_iter = FindTitle(item);
    auto item_iter = FindItem(title_iter, item);
    return std::pair{title_iter, item_iter};
  }

  void CheckReHash() {
    if (max_load_factor_ * data_.size() <= size_) {
      ReHash();
    }
  }

  void ReHash() {
    Vector<List<std::pair<const Key, Value>>> new_data(ComputeNewSize());
    for (auto &&list : data_) {
      for (auto &&item : list) {
        size_t id = hash_(item.GetItem().first) % new_data.size();
        new_data[id].PushFront(std::move(item.GetItem()));
      }
    }
    data_ = std::move(new_data);
  }

  size_t ComputeNewSize() {
    return std::max(size_t((data_.size()) * coefficient_), data_.size() + 1);
  }

  static const double max_load_factor_;
  static const double coefficient_;
  size_t size_ = 0;
  Hash hash_ = Hash();
  Vector<List<std::pair<const Key, Value>>> data_;
};

template <class Key, class Value, class Hash>
double const SimpleUnorderedMap<Key, Value, Hash>::max_load_factor_ = 3;

template <class Key, class Value, class Hash>
double const SimpleUnorderedMap<Key, Value, Hash>::coefficient_ = 2;