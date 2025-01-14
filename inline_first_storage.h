#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

template<size_t Capacity = 16> class inline_first_storage {
 public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = uint8_t;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using iterator = pointer;
  using const_iterator = const_pointer;

  static constexpr size_type inline_capacity = Capacity;

  inline_first_storage() noexcept : _capacity{inline_capacity}, _using_buffer{true}, _data{nullptr}
  {
  }

  inline_first_storage(const inline_first_storage &other)
      : _capacity(other._capacity), _using_buffer(other._capacity <= inline_capacity)
  {
    if (_using_buffer) {
      std::copy_n(other.data(), other.capacity(), _buffer);
    }
    else {
      _data = new value_type[other.capacity()];
      std::copy_n(other.data(), other.capacity(), _data);
    }
  }

  inline_first_storage(const inline_first_storage &&other) noexcept
      : _capacity(other._capacity), _using_buffer(other._capacity <= inline_capacity)
  {
    if (_using_buffer) {
      std::copy_n(other.data(), other.capacity(), _buffer);
    }
    else {
      _data = other._data;
      other._data = nullptr;
    }
    other._capacity = inline_first_storage::inline_capacity;
    other._using_buffer = true;
  }

  template<size_t OtherCapacity>
  inline_first_storage(const inline_first_storage<OtherCapacity> &other)
      : _capacity(std::max(inline_capacity, other._capacity)),
        _using_buffer(other._capacity <= inline_capacity)
  {
    if (_using_buffer) {
      std::copy_n(other.data(), other.capacity(), _buffer);
    }
    else {
      _data = new value_type[other.capacity()];
      std::copy_n(other.data(), other.capacity(), _data);
    }
  }

  template<size_t OtherCapacity>
  inline_first_storage(const inline_first_storage<OtherCapacity> &other, size_type copied_capacity)
      : _using_buffer(copied_capacity <= inline_capacity)
  {
    _capacity = std::max(inline_capacity, std::min(copied_capacity, other._capacity));
    if (_using_buffer) {
      std::copy_n(other.data(), copied_capacity, _buffer);
    }
    else {
      _data = new value_type[other.capacity()];
      std::copy_n(other.data(), copied_capacity, _data);
    }
  }

  template<size_t OtherCapacity>
  inline_first_storage(inline_first_storage<OtherCapacity> &&other) noexcept
      : _capacity(std::max(inline_capacity, other._capacity)),
        _using_buffer(other._capacity <= inline_capacity)
  {
    if (_using_buffer) {
      std::copy_n(other.data(), other.capacity(), _buffer);
    }
    else if (other._using_buffer) {
      _data = new value_type[other.capacity()];
      std::copy_n(other.data(), other.capacity(), _data);
    }
    else {
      _data = other._data;
      other._data = nullptr;
    }
    other.clear();
  }

  template<size_t OtherCapacity>
  inline_first_storage(inline_first_storage<OtherCapacity> &&other, size_type copied_capacity)
      : _using_buffer(copied_capacity <= inline_capacity)
  {
    _capacity = std::max(inline_capacity, std::min(copied_capacity, other._capacity));
    if (_using_buffer) {
      std::copy_n(other.data(), copied_capacity, _buffer);
    }
    else if (other._using_buffer) {
      _data = new value_type[other.capacity()];
      std::copy_n(other.data(), copied_capacity, _data);
    }
    else {
      _data = other._data;
      other._data = nullptr;
    }
    other.clear();
  }

  template<typename T>
    requires std::is_pointer_v<T>
  inline_first_storage(T other, size_type other_capacity)
      : _capacity(std::max(inline_capacity, other_capacity)),
        _using_buffer(other_capacity <= inline_capacity)
  {
    if (_using_buffer) {
      std::copy_n(other, other_capacity, _buffer);
    }
    else {
      _data = new value_type[other_capacity];
      std::copy_n(other, other_capacity, _data);
    }
  }

  ~inline_first_storage()
  {
    if (!_using_buffer) {
      delete[] data();
    }
  }

  size_type capacity() const
  {
    return _capacity;
  }

  void reserve(size_type new_capacity, bool no_copy = false)
  {
    if (new_capacity <= _capacity) {
      return;
    }

#ifdef _DEBUG
    if (new_capacity >= max_capacity()) {
      throw std::length_error("ustring::recapacity: capacity overflow");
    }
#endif

    value_type *new_data = new value_type[new_capacity];
    std::copy_n(data(), _capacity, new_data);
    if (!_using_buffer) {
      delete[] _data;
    }

    _data = new_data;
    _capacity = new_capacity;
    _using_buffer = false;
  }

  static constexpr size_type max_capacity()
  {
    return std::numeric_limits<size_type>::max() >> 1;
  }

  value_type &operator[](size_type index)
  {
    return data()[index];
  }

  const value_type &operator[](size_type index) const
  {
    return data()[index];
  }

  iterator begin()
  {
    return data();
  }
  const_iterator begin() const
  {
    return data();
  }
  iterator end()
  {
    return data() + _capacity;
  }
  const_iterator end() const
  {
    return data() + _capacity;
  }

  inline_first_storage &assign_from(const inline_first_storage &other, size_type size)
  {
    if (this != &other) {
      reserve(other._capacity, true);
      std::copy_n(other.data(), size, data());
    }
    return *this;
  }

  inline_first_storage &operator=(const inline_first_storage &other)
  {
    assign_from(other, other.capacity());
    return *this;
  }

  inline_first_storage &assign_from(inline_first_storage &&other, size_type size)
  {
    reserve(other._capacity, true);
    std::copy_n(other.data(), size, data());
    other._data = nullptr;
    other.clear();
    return *this;
  }

  inline_first_storage &operator=(inline_first_storage &&other) noexcept
  {
    assign_from(other, other.capacity());
    return *this;
  }

  pointer data()
  {
    return _using_buffer ? _buffer : _data;
  }

  const_pointer data() const
  {
    return _using_buffer ? _buffer : _data;
  }

  auto operator*()
  {
    return data();
  }

  auto operator*() const
  {
    return data();
  }

  std::tuple<const_pointer, size_type> to_span() const noexcept
  {
    return std::make_tuple(data(), capacity());
  }

  std::tuple<pointer, size_type> to_span() noexcept
  {
    return std::make_tuple(data(), capacity());
  }

  void clear()
  {
    if (!_using_buffer) {
      delete[] _data;
    }
    _capacity = inline_capacity;
    _using_buffer = true;
  }

  bool shrink(size_type size, bool copy_data = true, bool allow_fallback_to_inline = false) {
    if (size >= _capacity) {
      return false;
    }

    if (size <= inline_capacity && allow_fallback_to_inline) {
      if (!_using_buffer) {
        auto d = data();
        if (copy_data) {
          std::copy_n(d, size, _buffer);
        }
        delete[] d;
      }
      _using_buffer = true;
      _capacity = inline_capacity;
    } else if (!_using_buffer) {
      pointer new_data = new value_type[size];
      if (copy_data) {
        std::copy_n(data(), std::min(size, _capacity), new_data);
      }
      delete[] _data;
      _data = new_data;
      _capacity = size;
    }

    return true;
  }

  void reset(pointer data, size_type size)
  {
    if (!_using_buffer) {
      delete[] _data;
    }
    _data = data;
    _capacity = size;
    _using_buffer = false;
  }

 private:
  template<size_t OtherCapacity> friend class inline_first_storage;

  union {
    uint8_t *_data;
    uint8_t _buffer[inline_capacity];
  };
  struct {
    bool _using_buffer : 1;
    size_type _capacity : sizeof(size_type) * 8 - 1;
  };
};
