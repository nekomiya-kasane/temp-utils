//#pragma once
//#include <atomic>
//#include <cstring>
//#include <memory>
//#include <mutex>
//#include <thread>
//#include <type_traits>
//#include <utility>
//#include <vector>
//
//#if defined(__GXX_RTTI) || defined(__cpp_rtti) || defined(_CPPRTTI)
//#  define RTTI_ENABLED 1
//static constexpr bool rtti_enabled = true;
//#  include <typeinfo>
//#else
//static constexpr bool rtti_enabled = false;
//#endif
//
//namespace traits {
//
//// transformations
//template<typename T> std::weak_ptr<T> to_weak(std::weak_ptr<T> w)
//{
//  return w;
//}
//template<typename T> std::weak_ptr<T> to_weak(std::shared_ptr<T> w)
//{
//  return w;
//}
//
//// concepts
//template<typename F, typename... T>
//concept callable = requires(F f, T... args) { f(args...); };
//
//template<typename F, typename P, typename... T>
//concept member_callable = requires(F f, P p, T... args) { (p.*f)(args...); };
//
//template<typename T>
//concept has_function_call_operator = requires { &T::operator(); };
//
//template<typename T>
//concept weak_ptr = requires {
//  typename T::element_type;
//  requires std::same_as<T, std::weak_ptr<typename T::element_type>>;
//};
//
//template<typename T>
//concept shared_ptr = requires {
//  typename T::element_type;
//  requires std::same_as<T, std::shared_ptr<typename T::element_type>>;
//};
//
//template<typename T>
//concept weak_ptr_comparable = weak_ptr<T> || shared_ptr<T> || to_weak(std::declval<T>());
//
//// constexpr
//constexpr size_t max_size_of_function_ptr = [] {
//  struct a {
//    virtual ~a() = default;
//    void f();
//    virtual void g();
//    static void h();
//  };
//  struct b {
//    virtual ~b() = default;
//    void f();
//    virtual void g();
//  };
//  struct c : a, b {
//    void f();
//    void g() override;
//  };
//  struct d : virtual a {
//    void g() override;
//  };
//
//  return std::max({
//      sizeof(decltype(&d::g)),  // d 的虚函数指针
//      sizeof(decltype(&c::g)),  // c 的虚函数指针
//      sizeof(decltype(&a::f)),  // a 的普通成员函数指针
//      sizeof(decltype(&a::g)),  // a 的虚函数指针
//      sizeof(decltype(&a::h)),  // a 的静态成员函数指针
//      sizeof(void (*)()),       // 普通函数指针
//      sizeof(void *)            // 普通对象指针
//  });
//}();
//static_assert(max_size_of_function_ptr == 8 || max_size_of_function_ptr == 16,
//              "unexpected function pointer size");
//
//}  // namespace traits
//
//struct function_ptr {
//  function_ptr() : _size{0}
//  {
//    std::uninitialized_fill(_data, _data + traits::max_size_of_function_ptr, '\0');
//  }
//
//  template<typename T> void store(const T &t)
//  {
//    const auto *b = reinterpret_cast<const char *>(&t);
//    _size = sizeof(T);
//    std::memcpy(_data, b, _size);
//  }
//
//  template<typename T> const T *as() const
//  {
//    if (sizeof(T) != _size) {
//      return nullptr;
//    }
//    return reinterpret_cast<const T *>(_data);
//  }
//
// private:
//  alignas(traits::max_size_of_function_ptr) char _data[traits::max_size_of_function_ptr];
//  size_t _size;
//};
//
//namespace traits {
//
//template<typename T>
//  requires std::is_function_v<std::remove_pointer_t<T>>
//struct function_traits<T> {
//  static void set_ptr(T &t, function_ptr &d)
//  {
//    d.store(&t);
//  }
//
//  static bool is_same_ptr(T &t, const function_ptr &d)
//  {
//    const auto *r = d.as<const T *>();
//    return r && *r == &t;
//  }
//
//  static constexpr bool is_disconnectable = true;
//  static constexpr bool must_check_object = false;
//};
//
//template<typename T>
//  requires std::is_function_v<std::remove_pointer_t<T>>
//struct function_traits<T *> {
//  static void set_ptr(T *t, function_ptr &d)
//  {
//    function_traits<T>::ptr(*t, d);
//  }
//
//  static bool is_same_ptr(T *t, const function_ptr &d)
//  {
//    return function_traits<T>::eq(*t, d);
//  }
//
//  static constexpr bool is_disconnectable = true;
//  static constexpr bool must_check_object = false;
//};
//
//template<typename T>
//  requires std::is_member_function_pointer_v<T>
//struct function_traits<T> {
//  static void set_ptr(T t, function_ptr &d)
//  {
//    d.store(t);
//  }
//
//  static bool is_same_ptr(T t, const function_ptr &d)
//  {
//    const auto *r = d.as<const T>();
//    return r && *r == t;
//  }
//
//  static constexpr bool is_disconnectable = rtti_enabled;
//  static constexpr bool must_check_object = true;
//};
//
//template<typename T>
//  requires traits::has_function_call_operator<T>
//struct function_traits<T> {
//  using call_type = decltype(&std::remove_reference_t<T>::operator());
//
//  static void set_ptr(const T & /*t*/, function_ptr &d)
//  {
//    function_traits<call_type>::ptr(&T::operator(), d);
//  }
//
//  static bool is_same_ptr(const T & /*t*/, const function_ptr &d)
//  {
//    return function_traits<call_type>::eq(&T::operator(), d);
//  }
//
//  static constexpr bool is_disconnectable = function_traits<call_type>::is_disconnectable;
//  static constexpr bool must_check_object = function_traits<call_type>::must_check_object;
//};
//
//template<typename T> function_ptr get_function_ptr(const T &t)
//{
//  function_ptr d;
//  function_traits<std::decay_t<T>>::set_ptr(t, d);
//  return d;
//}
//
//template<typename T> bool is_same_ptr(const T &t, const function_ptr &d)
//{
//  return function_traits<std::decay_t<T>>::is_same_ptr(t, d);
//}
//
//}  // namespace traits
//
//using group_id = int32_t;
//
///* slot_state holds slot type independent state, to be used to interact with
// * slots indirectly through connection and scoped_connection objects.
// */
//class slot_state {
// public:
//  constexpr slot_state(group_id gid) noexcept
//      : m_index(0), m_group(gid), m_connected(true), m_blocked(false)
//  {
//  }
//
//  virtual ~slot_state() = default;
//
//  virtual bool connected() const noexcept
//  {
//    return m_connected;
//  }
//
//  bool disconnect() noexcept
//  {
//    bool ret = m_connected.exchange(false);
//    if (ret) {
//      sub_disconnect();
//    }
//    return ret;
//  }
//
//  bool blocked() const noexcept
//  {
//    return m_blocked.load();
//  }
//  void block() noexcept
//  {
//    m_blocked.store(true);
//  }
//  void unblock() noexcept
//  {
//    m_blocked.store(false);
//  }
//
// protected:
//  virtual void sub_disconnect() {}
//
//  auto index() const
//  {
//    return m_index;
//  }
//
//  auto &index()
//  {
//    return m_index;
//  }
//
//  group_id group() const
//  {
//    return m_group;
//  }
//
// private:
//  template<typename, typename...> friend class signal_base;
//
//  std::size_t m_index;     // index into the array of slot pointers inside the signal
//  const group_id m_group;  // slot group this slot belongs to
//  std::atomic<bool> m_connected;
//  std::atomic<bool> m_blocked;
//};
//
///**
// * A connection object allows interaction with an ongoing slot connection
// *
// * It allows common actions such as connection blocking and disconnection.
// * Note that connection is not a RAII object, one does not need to hold one
// * such object to keep the signal-slot connection alive.
// */
//class connection {
// public:
//  connection() = default;
//  virtual ~connection() = default;
//
//  connection(const connection &) noexcept = default;
//  connection &operator=(const connection &) noexcept = default;
//  connection(connection &&) noexcept = default;
//  connection &operator=(connection &&) noexcept = default;
//
//  bool valid() const noexcept
//  {
//    return !m_state.expired();
//  }
//
//  bool connected() const noexcept
//  {
//    const auto d = m_state.lock();
//    return d && d->connected();
//  }
//
//  bool disconnect() noexcept
//  {
//    auto d = m_state.lock();
//    return d && d->disconnect();
//  }
//
//  bool blocked() const noexcept
//  {
//    const auto d = m_state.lock();
//    return d && d->blocked();
//  }
//
//  void block() noexcept
//  {
//    if (auto d = m_state.lock()) {
//      d->block();
//    }
//  }
//
//  void unblock() noexcept
//  {
//    if (auto d = m_state.lock()) {
//      d->unblock();
//    }
//  }
//
//  connection_blocker blocker() const noexcept
//  {
//    return connection_blocker{m_state};
//  }
//
// protected:
//  template<typename, typename...> friend class signal_base;
//  explicit connection(std::weak_ptr<slot_state> s) noexcept : m_state{std::move(s)} {}
//
// protected:
//  std::weak_ptr<slot_state> m_state;
//};
