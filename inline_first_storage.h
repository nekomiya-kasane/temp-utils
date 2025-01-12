#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

std::vector<double> s;

template<size_t Size = 16, typename T = uint8_t, typename Allocator = std::allocator<T>>
class inline_first_storage {
 public:
  using value_type = T;
  using allocator_type = Allocator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = typename std::allocator_traits<Allocator>::pointer;
  using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
  using iterator = pointer;
  using const_iterator = const_pointer;

  static constexpr size_type inline_capacity = Size;

  inline_first_storage() noexcept : _size(0), _using_buffer(true) {}

  explicit inline_first_storage(const Allocator&) noexcept
      : _size(0), _using_buffer(true)
  {
  }

  inline_first_storage(const inline_first_storage &other)
      : _size(other._size),
        _using_buffer(other._using_buffer)
  {
    if (other._using_buffer) {
      std::copy_n(other.inline_data(), _size, inline_data());
    }
    else {
      pointer new_data = alloc_traits::allocate(get_allocator(), other._size);
      std::copy_n(other.data_, other._size, new_data);
      data_ = new_data;
    }
  }

  inline_first_storage(inline_first_storage &&other) noexcept
      : _size(other._size), _using_buffer(other._using_buffer)
  {
    if (_using_buffer) {
      std::copy_n(other.inline_data(), _size, inline_data());
    }
    else {
      data_ = other.data_;
      other.data_ = nullptr;
    }
    other._size = 0;
    other._using_buffer = true;
  }

  ~inline_first_storage()
  {
    if (!_using_buffer) {
      alloc_traits::deallocate(get_allocator(), data_, _size);
    }
  }

  inline_first_storage &operator=(const inline_first_storage &other)
  {
    if (this != &other) {
      assign(other.data(), other.size());
    }
    return *this;
  }

  inline_first_storage &operator=(inline_first_storage &&other) noexcept
  {
    if (this != &other) {
      clear();
      _size = other._size;
      _using_buffer = other._using_buffer;
      if (_using_buffer) {
        std::copy_n(other.inline_data(), _size, inline_data());
      }
      else {
        data_ = other.data_;
        other.data_ = nullptr;
      }
      other._size = 0;
      other._using_buffer = true;
    }
    return *this;
  }

  reference operator[](size_type pos) noexcept { return data()[pos]; }
  const_reference operator[](size_type pos) const noexcept { return data()[pos]; }
  
  reference at(size_type pos) {
    if (pos >= _size) throw std::out_of_range("inline_first_storage::at");
    return data()[pos];
  }
  
  const_reference at(size_type pos) const {
    if (pos >= _size) throw std::out_of_range("inline_first_storage::at");
    return data()[pos];
  }

  reference front() noexcept { return data()[0]; }
  const_reference front() const noexcept { return data()[0]; }
  reference back() noexcept { return data()[_size - 1]; }
  const_reference back() const noexcept { return data()[_size - 1]; }

  iterator begin() noexcept { return data(); }
  const_iterator begin() const noexcept { return data(); }
  const_iterator cbegin() const noexcept { return data(); }
  iterator end() noexcept { return data() + _size; }
  const_iterator end() const noexcept { return data() + _size; }
  const_iterator cend() const noexcept { return data() + _size; }

  [[nodiscard]] bool empty() const noexcept { return _size == 0; }
  size_type size() const noexcept { return _size; }
  size_type max_size() const noexcept { return alloc_traits::max_size(get_allocator()); }
  size_type capacity() const noexcept { return _using_buffer ? inline_capacity : _size; }

  void reserve(size_type new_cap)
  {
    if (new_cap <= capacity()) return;
    reallocate(new_cap);
  }

  void shrink_to_fit()
  {
    if (!_using_buffer && _size <= inline_capacity) {
      convert_to_inline();
    }
    else if (!_using_buffer) {
      reallocate(_size);
    }
  }

  void clear() noexcept
  {
    if (!_using_buffer) {
      alloc_traits::deallocate(get_allocator(), data_, _size);
      _using_buffer = true;
    }
    _size = 0;
  }

  void resize(size_type count) { resize(count, T()); }

  void resize(size_type count, const T &value)
  {
    if (count > _size) {
      reserve(count);
      std::fill_n(end(), count - _size, value);
    }
    _size = count;
  }

  template<typename... Args>
  reference emplace_back(Args&&... args)
  {
    if (_size == capacity()) {
      reserve(growth_size(_size + 1));
    }
    alloc_traits::construct(get_allocator(), data() + _size, std::forward<Args>(args)...);
    return data()[_size++];
  }

  void push_back(const T &value) { emplace_back(value); }
  void push_back(T &&value) { emplace_back(std::move(value)); }

  void pop_back()
  {
    if (!empty()) {
      --_size;
      if (!_using_buffer) {
        alloc_traits::destroy(get_allocator(), data() + _size);
      }
    }
  }

  pointer data() noexcept { return _using_buffer ? inline_data() : data_; }
  const_pointer data() const noexcept { return _using_buffer ? inline_data() : data_; }

  static allocator_type get_allocator() noexcept { return allocator_type(); }

 private:
  using alloc_traits = std::allocator_traits<Allocator>;

  union {
    pointer data_;
    std::array<T, inline_capacity> buffer_;
  };
  size_type _size;
  bool _using_buffer;

  pointer inline_data() noexcept { return reinterpret_cast<pointer>(buffer_.data()); }
  const_pointer inline_data() const noexcept { return reinterpret_cast<const_pointer>(buffer_.data()); }

  size_type growth_size(size_type required_size) const
  {
    const size_type ms = max_size();
    if (required_size > ms) throw std::length_error("inline_first_storage growth size overflow");
    const size_type new_size = std::max(required_size, _size * 2);
    return std::min(new_size, ms);
  }

  void reallocate(size_type new_cap)
  {
    pointer new_data = alloc_traits::allocate(get_allocator(), new_cap);
    
    if (_using_buffer) {
      std::copy_n(inline_data(), _size, new_data);
    }
    else {
      std::copy_n(data_, _size, new_data);
      alloc_traits::deallocate(get_allocator(), data_, _size);
    }
    
    data_ = new_data;
    _using_buffer = false;
  }

  void convert_to_inline()
  {
    std::array<T, inline_capacity> temp;
    std::copy_n(data_, _size, temp.data());
    alloc_traits::deallocate(get_allocator(), data_, _size);
    std::copy_n(temp.data(), _size, inline_data());
    _using_buffer = true;
  }

  void assign(const_pointer data, size_type count)
  {
    if (count <= inline_capacity) {
      if (!_using_buffer) {
        alloc_traits::deallocate(get_allocator(), data_, _size);
        _using_buffer = true;
      }
      std::copy_n(data, count, inline_data());
    }
    else {
      pointer new_data = alloc_traits::allocate(get_allocator(), count);
      std::copy_n(data, count, new_data);
      if (!_using_buffer) {
        alloc_traits::deallocate(get_allocator(), data_, _size);
      }
      data_ = new_data;
      _using_buffer = false;
    }
    _size = count;
  }
};
