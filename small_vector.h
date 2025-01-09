#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <ranges>
#include <span>
#include <stdexcept>
#include <type_traits>
// #include <immintrin.h>
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <execution>
#include <format>
#include <memory_resource>
#include <shared_mutex>

namespace traits {

template<typename U>
concept pointer_like = std::is_pointer_v<U> || requires(U u) {
  {
    u.operator->()
  } -> std::convertible_to<void *>;
  {
    *u
  } -> std::convertible_to<void>;
};

template<typename U>
concept deletable_pointer = pointer_like<U> && requires(U *p) { delete p; };

}  // namespace traits

template<typename T> class small_vector_view;

template<typename T, size_t N>
  requires N != 0
struct svector_storage {
  static_assert(N < 4096, "svector cannot be too large.");
  alignas(T) std::array<std::byte, sizeof(T) * N> inline_storage_;
};

template<typename T, size_t N>
  requires N == 0
struct svector_storage {
  
};

std::vector<int> s;

template<typename T, size_t N = 0, typename Allocator = std::allocator<T>> class small_vector {
 public:
  constexpr size_t buffer_size = N;
  using value_type = T;
  using allocator_type = Allocator;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = typename std::allocator_traits<Allocator>::pointer;
  using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

 private:
  alignas(T) std::array<std::byte, sizeof(T) * N> inline_storage_;
  pointer begin_ = reinterpret_cast<T *>(inline_storage_.data());
  pointer end_ = begin_;
  pointer capacity_end_ = begin_ + N;
  [[no_unique_address]] Allocator alloc_;

  bool is_using_buffer() const noexcept
  {
    return begin_ == reinterpret_cast<const T *>(inline_storage_.data());
  }

  void destroy_range(pointer first, pointer last)
  {
    while (first != last) {
      std::allocator_traits<Allocator>::destroy(alloc_, first);
      ++first;
    }
  }

  void deallocate_if_needed()
  {
    if (!is_using_buffer()) {
      std::allocator_traits<Allocator>::deallocate(alloc_, begin_, capacity_end_ - begin_);
    }
  }

  void grow(size_type new_capacity)
  {
    const size_type old_size = size();
    pointer new_storage = std::allocator_traits<Allocator>::allocate(alloc_, new_capacity);

    try {
      std::uninitialized_move_n(begin_, old_size, new_storage);
    }
    catch (...) {
      std::allocator_traits<Allocator>::deallocate(alloc_, new_storage, new_capacity);
      throw;
    }

    destroy_range(begin_, end_);
    deallocate_if_needed();

    begin_ = new_storage;
    end_ = new_storage + old_size;
    capacity_end_ = new_storage + new_capacity;
  }

 public:
  small_vector() noexcept = default;

  explicit small_vector(const Allocator &alloc) noexcept : alloc_(alloc) {}

  explicit small_vector(size_type count,
                        const T &value = T(),
                        const Allocator &alloc = Allocator())
      : alloc_(alloc)
  {
    if (count > N) {
      begin_ = std::allocator_traits<Allocator>::allocate(alloc_, count);
      capacity_end_ = begin_ + count;
    }
    end_ = begin_ + count;
    std::uninitialized_fill_n(begin_, count, value);
  }

  small_vector(const small_vector &other)
      : alloc_(
            std::allocator_traits<Allocator>::select_on_container_copy_construction(other.alloc_))
  {
    const size_type other_size = other.size();
    if (other_size > N) {
      begin_ = std::allocator_traits<Allocator>::allocate(alloc_, other_size);
      capacity_end_ = begin_ + other_size;
    }
    end_ = begin_ + other_size;
    std::uninitialized_copy_n(other.begin_, other_size, begin_);
  }

  small_vector(small_vector &&other) noexcept
  {
    if (other.is_using_buffer()) {
      std::uninitialized_move_n(other.begin_, other.size(), begin_);
      end_ = begin_ + other.size();
    }
    else {
      begin_ = other.begin_;
      end_ = other.end_;
      capacity_end_ = other.capacity_end_;
      other.begin_ = reinterpret_cast<T *>(other.inline_storage_.data());
      other.end_ = other.begin_;
      other.capacity_end_ = other.begin_ + N;
    }
  }

  ~small_vector()
  {
    destroy_range(begin_, end_);
    deallocate_if_needed();
  }

  small_vector &operator=(const small_vector &other)
  {
    if (this != &other) {
      small_vector tmp(other);
      swap(tmp);
    }
    return *this;
  }

  small_vector &operator=(small_vector &&other) noexcept
  {
    if (this != &other) {
      destroy_range(begin_, end_);
      deallocate_if_needed();

      if (other.is_using_buffer()) {
        begin_ = reinterpret_cast<T *>(inline_storage_.data());
        std::uninitialized_move_n(other.begin_, other.size(), begin_);
        end_ = begin_ + other.size();
        capacity_end_ = begin_ + N;
      }
      else {
        begin_ = other.begin_;
        end_ = other.end_;
        capacity_end_ = other.capacity_end_;
        other.begin_ = reinterpret_cast<T *>(other.inline_storage_.data());
        other.end_ = other.begin_;
        other.capacity_end_ = other.begin_ + N;
      }
    }
    return *this;
  }

  // Element access
  reference at(size_type pos)
  {
    if (pos >= size()) {
      throw std::out_of_range("small_vector::at");
    }
    return begin_[pos];
  }

  const_reference at(size_type pos) const
  {
    if (pos >= size()) {
      throw std::out_of_range("small_vector::at");
    }
    return begin_[pos];
  }

  reference operator[](size_type pos) noexcept
  {
    return begin_[pos];
  }
  const_reference operator[](size_type pos) const noexcept
  {
    return begin_[pos];
  }

  reference front() noexcept
  {
    return *begin_;
  }
  const_reference front() const noexcept
  {
    return *begin_;
  }
  reference back() noexcept
  {
    return *(end_ - 1);
  }
  const_reference back() const noexcept
  {
    return *(end_ - 1);
  }

  // Iterators
  iterator begin() noexcept
  {
    return begin_;
  }
  const_iterator begin() const noexcept
  {
    return begin_;
  }
  const_iterator cbegin() const noexcept
  {
    return begin_;
  }
  iterator end() noexcept
  {
    return end_;
  }
  const_iterator end() const noexcept
  {
    return end_;
  }
  const_iterator cend() const noexcept
  {
    return end_;
  }

  reverse_iterator rbegin() noexcept
  {
    return reverse_iterator(end());
  }
  const_reverse_iterator rbegin() const noexcept
  {
    return const_reverse_iterator(end());
  }
  reverse_iterator rend() noexcept
  {
    return reverse_iterator(begin());
  }
  const_reverse_iterator rend() const noexcept
  {
    return const_reverse_iterator(begin());
  }

  // Capacity
  [[nodiscard]] bool empty() const noexcept
  {
    return begin_ == end_;
  }
  size_type size() const noexcept
  {
    return end_ - begin_;
  }
  size_type max_size() const noexcept
  {
    return std::allocator_traits<Allocator>::max_size(alloc_);
  }
  size_type capacity() const noexcept
  {
    return capacity_end_ - begin_;
  }

  void reserve(size_type new_cap)
  {
    if (new_cap > capacity()) {
      grow(new_cap);
    }
  }

  void shrink_to_fit()
  {
    if (!is_using_buffer() && size() <= N) {
      small_vector tmp(*this);
      swap(tmp);
    }
  }

  // Modifiers
  void clear() noexcept
  {
    destroy_range(begin_, end_);
    end_ = begin_;
  }

  iterator insert(const_iterator pos, const T &value)
  {
    const auto offset = pos - begin_;
    if (end_ == capacity_end_) {
      const size_type new_cap = size() * 2 + 1;
      grow(new_cap);
    }
    const auto insert_pos = begin_ + offset;
    std::move_backward(insert_pos, end_, end_ + 1);
    *insert_pos = value;
    ++end_;
    return insert_pos;
  }

  template<std::input_iterator It> iterator insert(const_iterator pos, It first, It last)
  {
    const auto offset = pos - begin_;
    const auto count = std::distance(first, last);
    const auto new_size = size() + count;

    if (new_size > capacity()) {
      const size_type new_cap = std::max(new_size, size() * 2);
      grow(new_cap);
    }

    const auto insert_pos = begin_ + offset;
    std::move_backward(insert_pos, end_, end_ + count);
    std::copy(first, last, insert_pos);
    end_ += count;
    return insert_pos;
  }

  iterator erase(const_iterator pos)
  {
    const auto p = begin_ + (pos - begin_);
    std::move(p + 1, end_, p);
    --end_;
    std::allocator_traits<Allocator>::destroy(alloc_, end_);
    return iterator(p);
  }

  iterator erase(const_iterator first, const_iterator last)
  {
    const auto p = begin_ + (first - begin_);
    const auto new_end = std::move(begin_ + (last - begin_), end_, p);
    destroy_range(new_end, end_);
    end_ = new_end;
    return iterator(p);
  }

  void push_back(const T &value)
  {
    if (end_ == capacity_end_) {
      const size_type new_cap = size() * 2 + 1;
      grow(new_cap);
    }
    std::allocator_traits<Allocator>::construct(alloc_, end_, value);
    ++end_;
  }

  void push_back(T &&value)
  {
    if (end_ == capacity_end_) {
      const size_type new_cap = size() * 2 + 1;
      grow(new_cap);
    }
    std::allocator_traits<Allocator>::construct(alloc_, end_, std::move(value));
    ++end_;
  }

  template<typename... Args> reference emplace_back(Args &&...args)
  {
    if (end_ == capacity_end_) {
      const size_type new_cap = size() * 2 + 1;
      grow(new_cap);
    }
    std::allocator_traits<Allocator>::construct(alloc_, end_, std::forward<Args>(args)...);
    ++end_;
    return back();
  }

  void pop_back()
  {
    --end_;
    std::allocator_traits<Allocator>::destroy(alloc_, end_);
  }

  void resize(size_type count)
  {
    if (count > size()) {
      if (count > capacity()) {
        grow(count);
      }
      std::uninitialized_value_construct_n(end_, count - size());
    }
    else {
      destroy_range(begin_ + count, end_);
    }
    end_ = begin_ + count;
  }

  void resize(size_type count, const value_type &value)
  {
    if (count > size()) {
      if (count > capacity()) {
        grow(count);
      }
      std::uninitialized_fill_n(end_, count - size(), value);
    }
    else {
      destroy_range(begin_ + count, end_);
    }
    end_ = begin_ + count;
  }

  void swap(small_vector &other) noexcept
  {
    if (is_using_buffer() && other.is_using_buffer()) {
      const size_type min_size = std::min(size(), other.size());
      for (size_type i = 0; i < min_size; ++i) {
        std::swap(begin_[i], other.begin_[i]);
      }

      if (size() < other.size()) {
        std::uninitialized_move_n(other.begin_ + size(), other.size() - size(), end_);
        destroy_range(other.begin_ + size(), other.end_);
      }
      else if (other.size() < size()) {
        std::uninitialized_move_n(begin_ + other.size(), size() - other.size(), other.end_);
        destroy_range(begin_ + other.size(), end_);
      }
      std::swap(end_, other.end_);
    }
    else {
      std::swap(begin_, other.begin_);
      std::swap(end_, other.end_);
      std::swap(capacity_end_, other.capacity_end_);
    }
  }

  // Range support
  auto as_span() noexcept
  {
    return std::span(begin_, end_);
  }
  auto as_span() const noexcept
  {
    return std::span(begin_, end_);
  }

  //// SIMD 优化的求和操作
  // template<typename U = T>
  // requires (simd_traits<U>::is_vectorizable)
  // T sum() const noexcept {
  //     if (size() < simd_traits<T>::vector_size) {
  //         return std::accumulate(begin(), end(), T{});
  //     }

  //    using vector_type = typename simd_traits<T>::vector_type;
  //    const size_t vec_size = simd_traits<T>::vector_size;
  //    const size_t vec_count = size() / vec_size;
  //    const size_t remainder = size() % vec_size;

  //    vector_type sum_vec;
  //    if constexpr (std::is_same_v<T, float>) {
  //        sum_vec = _mm256_setzero_ps();
  //        for (size_t i = 0; i < vec_count; ++i) {
  //            sum_vec = _mm256_add_ps(sum_vec,
  //                _mm256_loadu_ps(begin() + i * vec_size));
  //        }
  //    } else if constexpr (std::is_same_v<T, double>) {
  //        sum_vec = _mm256_setzero_pd();
  //        for (size_t i = 0; i < vec_count; ++i) {
  //            sum_vec = _mm256_add_pd(sum_vec,
  //                _mm256_loadu_pd(begin() + i * vec_size));
  //        }
  //    } else if constexpr (std::is_same_v<T, int32_t>) {
  //        sum_vec = _mm256_setzero_si256();
  //        for (size_t i = 0; i < vec_count; ++i) {
  //            sum_vec = _mm256_add_epi32(sum_vec,
  //                _mm256_loadu_si256(reinterpret_cast<const __m256i*>(begin() + i * vec_size)));
  //        }
  //    }

  //    alignas(32) T result[vec_size];
  //    if constexpr (std::is_same_v<T, float>) {
  //        _mm256_store_ps(result, sum_vec);
  //    } else if constexpr (std::is_same_v<T, double>) {
  //        _mm256_store_pd(result, sum_vec);
  //    } else if constexpr (std::is_same_v<T, int32_t>) {
  //        _mm256_store_si256(reinterpret_cast<__m256i*>(result), sum_vec);
  //    }

  //    T final_sum = std::accumulate(result, result + vec_size, T{});
  //    return final_sum + std::accumulate(end() - remainder, end(), T{});
  //}

  template<typename Archive> void serialize(Archive &ar)
  {
    size_type s = size();
    ar & s;
    if (s > capacity()) {
      grow(s);
    }
    end_ = begin_ + s;
    for (size_type i = 0; i < s; ++i) {
      ar &operator[](i);
    }
  }

  // 获取原始数据指针
  [[nodiscard]] pointer data() noexcept
  {
    return begin_;
  }
  [[nodiscard]] const_pointer data() const noexcept
  {
    return begin_;
  }

  // 创建视图
  [[nodiscard]] small_vector_view<T> view() noexcept
  {
    return small_vector_view<T>(data(), size());
  }
  [[nodiscard]] small_vector_view<const T> view() const noexcept
  {
    return small_vector_view<const T>(data(), size());
  }

  // 批量操作
  template<typename UnaryOp> void transform_inplace(UnaryOp op)
  {
    std::transform(begin(), end(), begin(), std::move(op));
  }

  template<typename BinaryOp> void transform_inplace(const small_vector &other, BinaryOp op)
  {
    assert(size() == other.size());
    std::transform(begin(), end(), other.begin(), begin(), std::move(op));
  }

  // 高效的批量插入
  template<typename InputIt>
    requires std::input_iterator<InputIt>
  void insert_bulk(const_iterator pos, InputIt first, InputIt last)
  {
    const auto offset = pos - begin();
    const auto count = std::distance(first, last);
    const auto new_size = size() + count;

    if (new_size > capacity()) {
      const size_type new_cap = std::max(new_size, size() * 2);
      grow(new_cap);
    }

    const auto insert_pos = begin_ + offset;
    if constexpr (std::contiguous_iterator<InputIt> && std::is_trivially_copyable_v<T>) {
      // 使用 memmove 进行快速移动
      if (const auto move_count = end_ - insert_pos; move_count > 0) {
        std::memmove(insert_pos + count, insert_pos, move_count * sizeof(T));
      }
      std::memcpy(insert_pos, std::to_address(first), count * sizeof(T));
    }
    else {
      std::move_backward(insert_pos, end_, end_ + count);
      std::copy(first, last, insert_pos);
    }
    end_ += count;
  }

  // 内存对齐辅助类
  struct alignment_helper {
    static constexpr size_t value = std::max(std::max(alignof(T), alignof(std::max_align_t)),
                                             size_t(32)  // AVX 对齐要求
    );
  };

  // 高效的内存预留
  void reserve_exact(size_type new_cap)
  {
    if (new_cap <= capacity())
      return;

    pointer new_begin;
    if constexpr (std::is_trivially_copyable_v<T>) {
      new_begin = static_cast<pointer>(
#ifdef _WIN32
          _aligned_alloc
#else
          std::aligned_alloc
#endif
          (alignment_helper::value, new_cap * sizeof(T)));
      if (!new_begin)
        throw std::bad_alloc();

      std::memcpy(new_begin, begin_, size() * sizeof(T));
    }
    else {
      new_begin = std::allocator_traits<Allocator>::allocate(alloc_, new_cap);
      std::uninitialized_move_n(begin_, size(), new_begin);
    }

    destroy_range(begin_, end_);
    deallocate_if_needed();

    begin_ = new_begin;
    end_ = begin_ + size();
    capacity_end_ = begin_ + new_cap;
  }

  // 并行算法支持
  template<typename UnaryOp> void parallel_transform(UnaryOp op)
  {
    std::transform(std::execution::par_unseq, begin(), end(), begin(), std::move(op));
  }

  template<typename T2, std::size_t N2> void parallel_copy_from(const small_vector<T2, N2> &other)
  {
    const size_type other_size = other.size();
    reserve(other_size);
    std::transform(
        std::execution::par_unseq, other.begin(), other.end(), begin_, [](const auto &x) {
          return static_cast<T>(x);
        });
    end_ = begin_ + other_size;
  }

  // 自定义内存资源支持
  template<typename U = T>
    requires std::is_trivially_copyable_v<U>
  void set_memory_resource(std::pmr::memory_resource *resource)
  {
    if (size() == 0)
      return;

    std::pmr::polymorphic_allocator<T> new_alloc(resource);
    pointer new_begin = new_alloc.allocate(capacity());
    std::memcpy(new_begin, begin_, size() * sizeof(T));

    destroy_range(begin_, end_);
    deallocate_if_needed();

    begin_ = new_begin;
    end_ = begin_ + size();
    capacity_end_ = begin_ + capacity();
    alloc_ = new_alloc;
  }

  // 高级算法支持
  template<typename Pred> void stable_partition_inplace(Pred pred)
  {
    auto partition_point = std::stable_partition(begin(), end(), pred);
    size_type new_size = std::distance(begin(), partition_point);
    destroy_range(partition_point, end_);
    end_ = begin_ + new_size;
  }

  void remove_duplicates()
  {
    if (size() <= 1)
      return;
    std::sort(begin(), end());
    auto new_end = std::unique(begin(), end());
    destroy_range(new_end, end_);
    end_ = new_end;
  }

  // 批量操作优化
  template<typename Container>
    requires std::ranges::range<Container>
  void append_range(Container &&container)
  {
    insert_bulk(end(), std::ranges::begin(container), std::ranges::end(container));
  }

  void resize_uninitialized(size_type count)
  {
    if (count > capacity()) {
      reserve(count);
    }
    end_ = begin_ + count;
  }

  // 内存使用统计
  struct memory_stats {
    size_type size;
    size_type capacity;
    size_type inline_capacity;
    bool using_inline_storage;
    size_t allocated_bytes;
    size_t wasted_bytes;
  };

  memory_stats get_memory_stats() const noexcept
  {
    const size_type curr_size = size();
    const size_type curr_capacity = capacity();
    return memory_stats{.size = curr_size,
                        .capacity = curr_capacity,
                        .inline_capacity = N,
                        .using_inline_storage = is_using_buffer(),
                        .allocated_bytes = curr_capacity * sizeof(T),
                        .wasted_bytes = (curr_capacity - curr_size) * sizeof(T)};
  }

  void swap_elements(size_type i, size_type j) noexcept
  {
    assert(i < size() && j < size());
    if (i == j)
      return;
    if constexpr (std::is_trivially_copyable_v<T>) {
      T temp;
      std::memcpy(&temp, begin_ + i, sizeof(T));
      std::memcpy(begin_ + i, begin_ + j, sizeof(T));
      std::memcpy(begin_ + j, &temp, sizeof(T));
    }
    else {
      std::swap(begin_[i], begin_[j]);
    }
  }

  template<typename Pred> size_type count_if(Pred pred) const
  {
    return std::count_if(begin(), end(), pred);
  }

  template<typename T2> bool contains(const T2 &value) const
  {
    return std::find(begin(), end(), value) != end();
  }

  template<typename Pred> size_type remove_if(Pred pred)
  {
    auto new_end = std::remove_if(begin(), end(), pred);
    const size_type removed = std::distance(new_end, end());
    destroy_range(new_end, end_);
    end_ = new_end;
    return removed;
  }

  template<typename U = T>
    requires traits::deletable_pointer<U>
  void destroy_and_clear() noexcept
  {
    if constexpr (std::is_pointer_v<U>) {
      for (auto ptr : *this) {
        delete ptr;
      }
    }
    else {
      static_assert(std::is_pointer_v<U> ||
                        std::is_same_v<U, std::unique_ptr<typename U::element_type>> ||
                        std::is_same_v<U, std::shared_ptr<typename U::element_type>>,
                    "Type must be a raw pointer or smart pointer");
    }
    clear();
  }

  // 安全地删除所有数组指针并清空容器
  template<typename U = T>
    requires traits::deletable_pointer<U>
  void destroy_arrays_and_clear() noexcept
  {
    if constexpr (std::is_pointer_v<U>) {
      // 原始指针
      for (auto ptr : *this) {
        delete[] ptr;
      }
    }
    else {
      // 智能指针会自动释放
      static_assert(std::is_pointer_v<U> ||
                        std::is_same_v<U, std::unique_ptr<typename U::element_type[]>> ||
                        std::is_same_v<U, std::shared_ptr<typename U::element_type[]>>,
                    "Type must be a raw pointer or smart pointer to array");
    }
    clear();
  }

  // 使用自定义删除器删除所有指针并清空容器
  template<typename U = T, typename Deleter>
    requires traits::pointer_like<U>
  void destroy_and_clear_with(Deleter &&deleter) noexcept(
      noexcept(std::declval<Deleter>()(std::declval<U>())))
  {
    for (auto ptr : *this) {
      if (ptr != nullptr) {  // 额外的空指针检查
        deleter(ptr);
      }
    }
    clear();
  }
};

template<typename T> class small_vector_view {
  T *data_;
  size_t size_;

 public:
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = T &;
  using const_reference = const T &;
  using pointer = T *;
  using const_pointer = const T *;
  using iterator = T *;
  using const_iterator = const T *;

  constexpr small_vector_view() noexcept : data_(nullptr), size_(0) {}

  template<std::size_t N, typename Alloc>
  constexpr small_vector_view(small_vector<T, N, Alloc> &vec) noexcept
      : data_(vec.data()), size_(vec.size())
  {
  }

  constexpr small_vector_view(T *data, size_t size) noexcept : data_(data), size_(size) {}

  [[nodiscard]] constexpr iterator begin() noexcept
  {
    return data_;
  }
  [[nodiscard]] constexpr const_iterator begin() const noexcept
  {
    return data_;
  }
  [[nodiscard]] constexpr iterator end() noexcept
  {
    return data_ + size_;
  }
  [[nodiscard]] constexpr const_iterator end() const noexcept
  {
    return data_ + size_;
  }

  [[nodiscard]] constexpr size_type size() const noexcept
  {
    return size_;
  }
  [[nodiscard]] constexpr bool empty() const noexcept
  {
    return size_ == 0;
  }

  constexpr reference operator[](size_type pos) noexcept
  {
    assert(pos < size_);
    return data_[pos];
  }

  constexpr const_reference operator[](size_type pos) const noexcept
  {
    assert(pos < size_);
    return data_[pos];
  }

  auto as_span() noexcept
  {
    return std::span(data_, size_);
  }
  auto as_span() const noexcept
  {
    return std::span(data_, size_);
  }
};

// Deduction guides
template<typename InputIt,
         typename Alloc = std::allocator<typename std::iterator_traits<InputIt>::value_type>>
small_vector(InputIt, InputIt, Alloc = Alloc())
    -> small_vector<typename std::iterator_traits<InputIt>::value_type, 16, Alloc>;

// Non-member functions
template<typename T, std::size_t N, typename Alloc>
bool operator==(const small_vector<T, N, Alloc> &lhs, const small_vector<T, N, Alloc> &rhs)
{
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename T, std::size_t N, typename Alloc>
auto operator<=>(const small_vector<T, N, Alloc> &lhs, const small_vector<T, N, Alloc> &rhs)
{
  return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

// Specialized algorithms
template<typename T, std::size_t N, typename Alloc>
void swap(small_vector<T, N, Alloc> &lhs, small_vector<T, N, Alloc> &rhs) noexcept
{
  lhs.swap(rhs);
}

// 内存对齐辅助类
template<typename T> struct alignment_helper {
  static constexpr size_t value = std::max(std::max(alignof(T), alignof(std::max_align_t)),
                                           size_t(32)  // AVX 对齐要求
  );
};

// 线程安全的小向量
template<typename T, std::size_t N, typename Allocator = std::allocator<T>>
class concurrent_small_vector {
  mutable std::shared_mutex mutex_;
  small_vector<T, N, Allocator> data_;

 public:
  void push_back(const T &value)
  {
    std::unique_lock lock(mutex_);
    data_.push_back(value);
  }

  template<typename... Args> void emplace_back(Args &&...args)
  {
    std::unique_lock lock(mutex_);
    data_.emplace_back(std::forward<Args>(args)...);
  }

  T operator[](size_t index) const
  {
    std::shared_lock lock(mutex_);
    return data_[index];
  }

  size_t size() const
  {
    std::shared_lock lock(mutex_);
    return data_.size();
  }

  template<typename F> void atomic_update(F &&func)
  {
    std::unique_lock lock(mutex_);
    func(data_);
  }
};
