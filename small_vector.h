#pragma once

#include "inline_first_storage.h"
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <format>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <type_traits>

template<typename T, size_t InlineCapacity = 16> class small_vector {
 public:
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  static constexpr size_type inline_capacity = InlineCapacity;

  constexpr small_vector() noexcept = default;

  constexpr explicit small_vector(size_type count) : _size(count)
  {
    _storage.reserve(count * sizeof(T));
    std::uninitialized_value_construct_n(data(), count);
  }

  constexpr small_vector(size_type count, const T &value) : _size(count)
  {
    _storage.reserve(count * sizeof(T));
    std::uninitialized_fill_n(data(), count, value);
  }

  template<std::input_iterator InputIt>
  constexpr small_vector(InputIt first, InputIt last)
    requires std::convertible_to<std::iter_value_t<InputIt>, T>
  {
    size_type count = std::distance(first, last);
    _storage.reserve(count * sizeof(T));
    std::uninitialized_copy(first, last, data());
    _size = count;
  }

  constexpr small_vector(const small_vector &other) : _size(other._size)
  {
    _storage.reserve(other._size * sizeof(T));
    std::uninitialized_copy_n(other.data(), other._size, data());
  }

  constexpr small_vector(small_vector &&other) noexcept
      : _storage(std::move(other._storage)), _size(other._size)
  {
    other._size = 0;
  }

  constexpr small_vector(std::initializer_list<T> init) : _size(init.size())
  {
    _storage.reserve(init.size() * sizeof(T));
    std::uninitialized_copy(init.begin(), init.end(), data());
  }

  constexpr ~small_vector()
  {
    // nothing to do, storage will release itself
  }

  constexpr small_vector &operator=(const small_vector &other)
  {
    if (this != &other) {
      assign(other.cbegin(), other.cend());
    }
    return *this;
  }

  constexpr small_vector &operator=(small_vector &&other) noexcept
  {
    if (this != &other) {
      _storage = std::move(other._storage);
      _size = other._size;
      other._size = 0;
    }
    return *this;
  }

  constexpr small_vector &operator=(std::initializer_list<T> ilist)
  {
    assign(ilist.cbegin(), ilist.cend());
    return *this;
  }

  constexpr void assign(size_type count, const T &value)
  {
    clear();
    if (count > 0) {
      _storage.reserve(count * sizeof(T));
      std::uninitialized_fill_n(data(), count, value);
      _size = count;
    }
  }

  template<std::input_iterator InputIt>
  constexpr void assign(InputIt first, InputIt last)
    requires std::convertible_to<std::iter_value_t<InputIt>, T>
  {
    clear();
    size_type count = std::distance(first, last);
    if (count > 0) {
      _storage.reserve(count * sizeof(T));
      std::uninitialized_copy(first, last, data());
      _size = count;
    }
  }

  constexpr void assign(std::initializer_list<T> ilist)
  {
    assign(ilist.begin(), ilist.end());
  }

  // Element access
  [[nodiscard]] constexpr reference at(size_type pos)
  {
    if (pos >= _size)
      throw std::out_of_range("small_vector::at");
    return data()[pos];
  }

  [[nodiscard]] constexpr const_reference at(size_type pos) const
  {
    if (pos >= _size)
      throw std::out_of_range("small_vector::at");
    return data()[pos];
  }

  constexpr reference operator[](size_type pos) noexcept
  {
    return data()[pos];
  }

  constexpr const_reference operator[](size_type pos) const noexcept
  {
    return data()[pos];
  }

  constexpr reference front() noexcept
  {
    return data()[0];
  }

  constexpr const_reference front() const noexcept
  {
    return data()[0];
  }

  constexpr reference back() noexcept
  {
    return data()[_size - 1];
  }

  constexpr const_reference back() const noexcept
  {
    return data()[_size - 1];
  }

  constexpr T *data() noexcept
  {
    return reinterpret_cast<T *>(_storage.data());
  }

  constexpr const T *data() const noexcept
  {
    return reinterpret_cast<const T *>(_storage.data());
  }

  // Iterators
  [[nodiscard]] constexpr iterator begin() noexcept
  {
    return data();
  }

  [[nodiscard]] constexpr const_iterator cbegin() const noexcept
  {
    return data();
  }

  [[nodiscard]] constexpr iterator end() noexcept
  {
    return data() + _size;
  }

  [[nodiscard]] constexpr const_iterator cend() const noexcept
  {
    return data() + _size;
  }

  [[nodiscard]] constexpr reverse_iterator rbegin() noexcept
  {
    return reverse_iterator(end());
  }

  [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
  {
    return const_reverse_iterator(end());
  }

  [[nodiscard]] constexpr reverse_iterator rend() noexcept
  {
    return reverse_iterator(begin());
  }

  [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
  {
    return const_reverse_iterator(begin());
  }

  // Enable view interface
  template<std::ranges::range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, T>
  constexpr small_vector &operator=(R &&r)
  {
    assign(std::ranges::begin(r), std::ranges::end(r));
    return *this;
  }

  //template<std::ranges::range R>
  //  requires std::convertible_to<std::ranges::range_value_t<R>, T>
  //constexpr small_vector(R &&r)
  //{
  //  assign(std::ranges::begin(r), std::ranges::end(r));
  //}

  // Capacity
  [[nodiscard]] constexpr bool empty() const noexcept
  {
    return _size == 0;
  }

  [[nodiscard]] constexpr size_type size() const noexcept
  {
    return _size;
  }

  [[nodiscard]] constexpr size_type max_size() const noexcept
  {
    return _storage.max_capacity() / sizeof(T);
  }

  constexpr void reserve(size_type new_cap)
  {
    if (new_cap > capacity()) {
      _storage.reserve(new_cap * sizeof(T));
    }
  }

  [[nodiscard]] constexpr size_type capacity() const noexcept
  {
    return _storage.capacity() / sizeof(T);
  }

  constexpr void shrink_to_fit()
  {
    if (_size < capacity()) {
      _storage.shrink(_size * sizeof(T), true, true);
    }
  }

  // Modifiers
  constexpr void clear() noexcept
  {
    std::destroy_n(data(), _size);
    _storage.clear();
    _size = 0;
  }

  constexpr iterator insert(const_iterator pos, const T &value)
  {
    return insert(pos, 1, value);
  }

  constexpr iterator insert(const_iterator pos, T &&value)
  {
    size_type index = pos - begin();
    if (_size == capacity()) {
      reserve(_size * 3 / 2);
    }
    iterator p = begin() + index;
    // todo: improve this
    if constexpr (std::is_default_constructible_v<T>) {
      std::move_backward(p, end(), end() + 1);
    }
    else {
      memmove(p + 1, p, (end() - p) * sizeof(T));
    }
    std::construct_at(p, std::move(value));
    ++_size;
    return p;
  }

  constexpr iterator insert(const_iterator pos, size_type count, const T &value)
  {
    size_type index = pos - begin();
    if (count > 0) {
      reserve(_size + count);
      iterator p = begin() + index;
      // todo: improve this
      if constexpr (std::is_default_constructible_v<T>) {
        std::uninitialized_value_construct_n(end(), count);
        std::move_backward(p, end(), end() + count);
      }
      else {
        memmove(p + count, p, (end() - p) * sizeof(T));
      }
      std::uninitialized_fill_n(p, count, value);
      _size += count;
    }
    return begin() + index;
  }

  template<std::input_iterator InputIt>
  constexpr iterator insert(const_iterator pos, InputIt first, InputIt last)
    requires std::convertible_to<std::iter_value_t<InputIt>, T>
  {
    size_type count = std::distance(first, last);
    if (!count) {
      return begin() + std::distance(pos, cbegin());
    }

    size_type index = pos - begin();
    if (count > 0) {
      reserve(_size + count);
      iterator p = begin() + index;
      // todo: improve this
      if constexpr (std::is_default_constructible_v<T>) {
        std::uninitialized_value_construct_n(end(), count);
        std::move_backward(p, end(), end() + count);
      }
      else {
        memmove(p + count, p, (end() - p) * sizeof(T));
      }
      std::uninitialized_copy(first, last, p);
      _size += count;
    }
    return begin() + index;
  }

  constexpr iterator insert(const_iterator pos, std::initializer_list<T> ilist)
  {
    return insert(pos, ilist.begin(), ilist.end());
  }

  template<typename... Args> constexpr iterator emplace(const_iterator pos, Args &&...args)
  {
    size_type index = pos - begin();
    if (_size == capacity()) {
      reserve(_size > 1 ? _size * 3 / 2 : _size + 1);
    }
    // todo: improve this
    iterator p = begin() + index;
    if constexpr (std::is_default_constructible_v<T>) {
      std::uninitialized_value_construct_n(end(), 1);
      std::move_backward(p, end(), end() + 1);
    }
    else {
      memmove(p + 1, p, (end() - p) * sizeof(T));
    }
    std::construct_at(p, std::forward<Args>(args)...);
    ++_size;
    return p;
  }

  constexpr iterator erase(const_iterator pos)
  {
    return erase(pos, pos + 1);
  }

  constexpr iterator erase(const_iterator first, const_iterator last)
  {
    iterator pos = begin() + (first - begin());
    if (first != last) {
      iterator new_end = std::move(begin() + (last - begin()), end(), pos);
      std::destroy(new_end, end());
      _size -= last - first;
    }
    return pos;
  }

  constexpr void push_back(const T &value)
  {
    emplace_back(value);
  }

  constexpr void push_back(T &&value)
  {
    emplace_back(std::move(value));
  }

  template<typename... Args> constexpr reference emplace_back(Args &&...args)
  {
    if (_size == capacity()) {
      reserve(_size > 1 ? _size * 3 / 2 : _size + 1);
    }
    std::construct_at(data() + _size, std::forward<Args>(args)...);
    return data()[_size++];
  }

  constexpr void pop_back()
  {
    std::destroy_at(data() + --_size);
  }

  constexpr void resize(size_type count)
  {
    if (count > _size) {
      reserve(count);
      std::uninitialized_value_construct_n(data() + _size, count - _size);
    }
    else if (count < _size) {
      std::destroy_n(data() + count, _size - count);
    }
    _size = count;
  }

  constexpr void resize(size_type count, const value_type &value)
  {
    if (count > _size) {
      reserve(count);
      std::uninitialized_fill_n(data() + _size, count - _size, value);
    }
    else if (count < _size) {
      std::destroy_n(data() + count, _size - count);
    }
    _size = count;
  }

  constexpr void swap(small_vector &other) noexcept
  {
    std::swap(_storage, other._storage);
    std::swap(_size, other._size);
  }

  friend constexpr bool operator==(const small_vector &lhs, const small_vector &rhs) noexcept
  {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
  }

  // Convert to std::vector
  operator std::vector<T>() const
  {
    return std::vector<T>(cbegin(), cend());
  }

  // Explicit conversion to std::vector
  std::vector<T> to_vector() const
  {
    return std::vector<T>(*this);
  }

  // View support
  class view {
   public:
    using value_type = T;
    using size_type = size_type;
    using difference_type = difference_type;
    using reference = const value_type &;
    using const_reference = const value_type &;
    using pointer = const value_type *;
    using const_pointer = const value_type *;
    using iterator = const value_type *;
    using const_iterator = const value_type *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using viewed_type = small_vector;

    constexpr view() noexcept = default;
    constexpr view(const small_vector &v) noexcept : _data(v.data()), _size(v.size()) {}
    constexpr view(const_pointer data, size_type size) noexcept : _data(data), _size(size) {}
    constexpr view(const_pointer start, const_pointer end) noexcept
        : _data(start), _size(std::distance(start, end))
    {
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
      return _data;
    }
    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
      return _data + _size;
    }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
      return begin();
    }
    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
      return end();
    }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
    {
      return const_reverse_iterator(end());
    }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
    {
      return const_reverse_iterator(begin());
    }
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
    {
      return rbegin();
    }
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
    {
      return rend();
    }

    [[nodiscard]] constexpr const_reference operator[](size_type pos) const
    {
      return _data[pos];
    }
    [[nodiscard]] constexpr const_reference at(size_type pos) const
    {
      if (pos >= _size) {
        throw std::out_of_range("small_vector::view::at");
      }
      return _data[pos];
    }
    [[nodiscard]] constexpr const_reference front() const
    {
      return _data[0];
    }
    [[nodiscard]] constexpr const_reference back() const
    {
      return _data[_size - 1];
    }
    [[nodiscard]] constexpr const_pointer data() const noexcept
    {
      return _data;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
      return _size == 0;
    }
    [[nodiscard]] constexpr size_type size() const noexcept
    {
      return _size;
    }
    [[nodiscard]] constexpr size_type max_size() const noexcept
    {
      return _size;
    }

    constexpr void remove_prefix(size_type n)
    {
      if (n > _size) {
        n = _size;
      }
      _data += n;
      _size -= n;
    }

    constexpr void remove_suffix(size_type n)
    {
      if (n > _size) {
        n = _size;
      }
      _size -= n;
    }

    constexpr void swap(view &other) noexcept
    {
      std::swap(_data, other._data);
      std::swap(_size, other._size);
    }

    [[nodiscard]] operator std::vector<T>() const
    {
      return std::vector<T>(begin(), end());
    }

    [[nodiscard]] std::vector<T> to_vector() const
    {
      return std::vector<T>(*this);
    }

    [[nodiscard]] friend constexpr bool operator==(const view &lhs, const view &rhs) noexcept
    {
      return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

   private:
    const_pointer _data{nullptr};
    size_type _size{0};
  };

  [[nodiscard]] constexpr view to_view() const noexcept
  {
    return view(*this);
  }

 private:
  inline_first_storage<InlineCapacity * sizeof(T)> _storage;
  size_type _size{0};
};

template<typename T, size_t N> using small_vector_view = typename small_vector<T, N>::view;

namespace traits {

template<class T> constexpr inline bool is_small_vector_v = false;

template<class T, std::size_t N>
constexpr inline bool is_small_vector_v<small_vector<T, N>> = true;

}  // namespace traits

namespace concepts {

template<class T>
concept small_vector_view = requires() {
  // has a small_vector_type alias
  typename T::viewed_type;
  // small_vector_type is a small_vector
  requires traits::is_small_vector_v<typename T::viewed_type>;
  // small_vector_type::view is equivalent to T
  requires std::same_as<typename T::viewed_type::view, T>;
};

}  // namespace concepts

// -> See: https://stackoverflow.com/questions/79355904/why-this-template-wont-compile
//
// This is actually a workaround as sometimes you cannot use nested templates like
// template<typename T, size_t N>
// struct std::formatter<small_vector_view<T, N>, char> {};
//                      \                      /
//                       ------- nested -------
//
template<concepts::small_vector_view T> struct std::formatter<T, char> {
  enum class Style { Default, Compact, Pretty };
  Style style = Style::Default;

  template<typename FormatParseContext> constexpr auto parse(FormatParseContext &ctx)
  {
    auto it = ctx.begin();
    if (it == ctx.end() || *it == '}')
      return it;

    switch (*it) {
      case 'c':
        style = Style::Compact;
        break;
      case 'p':
        style = Style::Pretty;
        break;
      default:
        throw format_error("invalid format specifier for small_vector");
    }
    return ++it;
  }

  template<typename FormatContext> auto format(const T &v, FormatContext &ctx) const
  {
    if (v.empty()) {
      if (style == Style::Pretty) {
        return format_to(ctx.out(), "[\n]");
      }
      return format_to(ctx.out(), "[]");
    }

    std::string result;
    if (style == Style::Pretty) {
      result = "[\n  ";
    }
    else {
      result = "[";
    }

    for (size_t i = 0; i < v.size(); ++i) {
      if (i > 0) {
        if (style == Style::Pretty) {
          result += ",\n  ";
        }
        else if (style == Style::Compact) {
          result += ",";
        }
        else {
          result += ", ";
        }
      }
      if constexpr (std::is_same_v<typename T::value_type, std::string>) {
        result += std::format("\"{}\"", v[i]);
      }
      else {
        result += std::format("{}", v[i]);
      }
    }

    if (style == Style::Pretty) {
      result += "\n]";
    }
    else {
      result += "]";
    }

    return format_to(ctx.out(), "{}", result);
  }
};

template<typename T, size_t N>
struct std::formatter<small_vector<T, N>>  // NOLINT(cert-dcl58-cpp)
    : std::formatter<small_vector_view<T, N>> {
  auto format(const small_vector<T, N> &v, format_context &ctx) const
  {
    return formatter<small_vector_view<T, N>>::format(
        small_vector_view<T, N>(v.cbegin(), v.cend()), ctx);
  }
};

template<typename T, size_t N>
std::ostream &operator<<(std::ostream &os, const small_vector<T, N> &v)
{
  os << std::format("{}", v);
  return os;
}

template<typename T, size_t N>
std::ostream &operator<<(std::ostream &os, const small_vector_view<T, N> &v)
{
  os << std::format("{}", v);
  return os;
}

template<typename T, size_t N>
std::ostringstream &operator<<(std::ostringstream &os, const small_vector<T, N> &v)
{
  os << std::format("{}", v);
  return os;
}

template<typename T, size_t N>
std::ostringstream &operator<<(std::ostringstream &os, const small_vector_view<T, N> &v)
{
  os << std::format("{}", v);
  return os;
}

template<typename T, size_t N> std::ostream &operator<<(std::ostream &os, small_vector<T, N> &&v)
{
  os << std::format("{}", v);
  return os;
}

template<typename T, size_t N>
std::ostream &operator<<(std::ostream &os, small_vector_view<T, N> &&v)
{
  os << std::format("{}", v);
  return os;
}

template<typename T, size_t N>
std::ostringstream &operator<<(std::ostringstream &os, small_vector<T, N> &&v)
{
  os << std::format("{}", v);
  return os;
}

template<typename T, size_t N>
std::ostringstream &operator<<(std::ostringstream &os, small_vector_view<T, N> &&v)
{
  os << std::format("{}", v);
  return os;
}
