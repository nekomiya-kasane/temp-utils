#pragma once

#include <format>
#include <source_location>
#include <tuple>
#include <type_traits>

namespace traits {
  /**
   * @~english
   * @brief Get the function name using source_location
   * @return The function name as a string_view
   *
   * @~chinese
   * @brief 使用source_location获取函数名
   * @return 以string_view形式返回函数名
   */
  template<typename = void> consteval auto function_name()
  {
    return std::source_location::current().function_name();
  }

  /**
   * @~english
   * @brief Get the type of an argument
   * @param arg The argument to get the type of
   * @return The type of the argument as a string_view
   *
   * @~chinese
   * @brief 获取参数的类型
   * @param arg 要获取类型的参数
   * @return 以string_view形式返回参数的类型
   */
  template<typename T> constexpr auto type_of(T &&arg)
  {
    constexpr std::string_view fmt = function_name<decltype(arg)>();
    return fmt.substr(41, fmt.size() - 47);
  }

  /**
   * @~english
   * @brief Get the type of a template parameter
   * @return The type of the template parameter as a string_view
   *
   * @~chinese
   * @brief 获取模板参数的类型
   * @return 以string_view形式返回模板参数的类型
   */
  template<typename T> constexpr auto type_of()
  {
    constexpr std::string_view fmt = function_name<T>();
    return fmt.substr(41, fmt.size() - 47);
  }

  /**
   * @~english
   * @brief Get the bare type of an argument
   * @param arg The argument to get the bare type of
   * @return The bare type of the argument as a string_view
   *
   * @~chinese
   * @brief 获取参数的原始类型
   * @param arg 要获取原始类型的参数
   * @return 以string_view形式返回参数的原始类型
   */
  template<typename T> constexpr auto bare_type_of(T &&arg)
  {
    constexpr std::string_view fmt = function_name<decltype(arg)>();
    return fmt.substr(42, fmt.size() - 49);
  }

  /**
   * @~english
   * @brief Get the bare type of a template parameter
   * @return The bare type of the template parameter as a string_view
   *
   * @~chinese
   * @brief 获取模板参数的原始类型
   * @return 以string_view形式返回模板参数的原始类型
   */
  template<typename T> constexpr auto bare_type_of()
  {
    constexpr std::string_view fmt = function_name<T>();
    return fmt.substr(42, fmt.size() - 49);
  }

  /**
   * @~english
   * @brief Concept to check if a type is formattable
   *
   * @~chinese
   * @brief 检查类型是否可格式化的概念
   */
  template<typename T>
  concept formattable = requires(std::format_context ctx, T value) {
    std::formatter<std::remove_cvref_t<T>>{}.format(value, ctx);
  };

  /**
   * @~english
   * @brief Concept to check if a type is a ranged type
   *
   * @~chinese
   * @brief 检查类型是否为范围类型的概念
   */
  template<typename T>
  concept ranged_type = requires(T t) {
    std::begin(t);
    std::end(t);
    std::ranges::subrange(std::begin(t), std::end(t));
  };

  /**
   * @~english
   * @brief Concept to check if a type is callable
   *
   * @~chinese
   * @brief 检查类型是否可调用的概念
   */
  template<typename T>
  concept callable = requires(T t) { (std::void_t<decltype(&T::operator())>)t; };

  /**
   * @~english
   * @brief Concept to check if two types are different
   *
   * @~chinese
   * @brief 检查类型是否不同
   */
  template<typename T, typename S> constexpr bool is_different_v = !std::is_same_v<T, S>;

  /**
   * @~english
   * @brief Enumeration for function types
   *
   * @~chinese
   * @brief 函数类型的枚举
   */
  enum class function_type {
    free_function = 0,    // Free function / 自由函数
    member_function = 1,  // Member function / 成员函数
    lambda = 2,           // Lambda function / Lambda 函数
    functor = 3           // Function object / 函数对象
  };

  /**
   * @~english
   * @brief Enumeration for function qualifiers
   *
   * @~chinese
   * @brief 函数限定符的枚举
   */
  enum class function_qualifiers : uint32_t {
    none = 0,
    is_const = 1 << 0,     // const member function / const 成员函数
    is_volatile = 1 << 1,  // volatile member function / volatile 成员函数
    is_lvalue = 1 << 2,    // lvalue reference qualifier & / 左值引用限定符 &
    is_rvalue = 1 << 3,    // rvalue reference qualifier && / 右值引用限定符 &&
    is_variadic = 1 << 4,  // variadic function / 可变参数函数
    is_noexcept = 1 << 5,  // noexcept function / noexcept 函数
    has_capture = 1 << 6,  // lambda with capture / lambda 带有捕获
    is_mutable = 1 << 7,   // mutable lambda / mutable lambda
  };

  // 位运算操作符重载
  constexpr function_qualifiers operator|(function_qualifiers a, function_qualifiers b)
  {
    return static_cast<function_qualifiers>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
  }

  constexpr function_qualifiers operator&(function_qualifiers a, function_qualifiers b)
  {
    return static_cast<function_qualifiers>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
  }

  constexpr bool has_qualifier(function_qualifiers flags, function_qualifiers qualifier)
  {
    return static_cast<uint32_t>(flags & qualifier) != 0;
  }

  /// @~english
  /// @brief Base function_traits template
  /// @tparam T The function type to analyze
  ///
  /// @~chinese
  /// @brief 基础 function_traits 模板
  /// @tparam T 要分析的函数类型
  template<typename T> struct function_traits {};

  /// @~english
  /// @brief Specialization for regular functions
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief 普通函数特化
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename R, typename... Args> struct function_traits<R (*)(Args...)> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = void;
    static constexpr function_type type = function_type::free_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::none;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for variadic functions
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief 可变参数普通函数特化
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename R, typename... Args> struct function_traits<R (*)(Args..., ...)> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = void;
    static constexpr function_type type = function_type::free_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for noexcept functions
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief noexcept 函数特化
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename R, typename... Args> struct function_traits<R (*)(Args...) noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = void;
    static constexpr function_type type = function_type::free_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for noexcept variadic functions
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief noexcept 可变参数函数特化
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename R, typename... Args> struct function_traits<R (*)(Args..., ...) noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = void;
    static constexpr function_type type = function_type::free_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_noexcept |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args> struct function_traits<R (C::*)(Args...)> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::none;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for volatile member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief volatile 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) volatile> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const volatile member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const volatile 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const volatile> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for lvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief 左值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args> struct function_traits<R (C::*)(Args...) &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_lvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const lvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const 左值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_lvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for volatile lvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief volatile 左值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) volatile &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_lvalue;
    static constexpr size_t arity = sizeof...(Args);
  };
  /// @~english
  /// @brief Specialization for const volatile lvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const volatile 左值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const volatile &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_lvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for rvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief 右值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args> struct function_traits<R (C::*)(Args...) &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_rvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const rvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const 右值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_rvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for volatile rvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief volatile 右值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) volatile &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_rvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const volatile rvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const volatile 右值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const volatile &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_rvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for rvalue reference variadic member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief 右值引用可变参数成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for variadic member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief 可变参数成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...)> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const variadic member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const 可变参数成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for volatile variadic member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief volatile 可变参数成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) volatile> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const volatile variadic member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const volatile 可变参数成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const volatile> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for volatile noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief volatile noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) volatile noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const volatile noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const volatile noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const volatile noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for lvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief 左值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };
  /// @~english
  /// @brief Specialization for const lvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const 左值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for volatile lvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief volatile 左值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) volatile & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const volatile lvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const volatile 左值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const volatile & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for rvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief 右值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const rvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const 右值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for volatile rvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief volatile 右值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) volatile && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const volatile rvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const volatile 右值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const volatile && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for variadic noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief 可变参数 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const variadic noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const 可变参数 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for volatile variadic noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief volatile 可变参数 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) volatile noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const volatile variadic noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const volatile 可变参数 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const volatile noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for variadic lvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief 可变参数左值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };
  /// @~english
  /// @brief Specialization for variadic lvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief 可变参数左值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const variadic lvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const 可变参数左值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for volatile variadic lvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief volatile 可变参数左值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) volatile & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const volatile variadic lvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const volatile 可变参数左值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const volatile & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for variadic rvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief 可变参数右值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const variadic rvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const 可变参数右值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for volatile variadic rvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief volatile 可变参数右值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) volatile && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const volatile variadic rvalue reference noexcept member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const volatile 可变参数右值引用 noexcept 成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const volatile && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const variadic lvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const 可变参数左值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const variadic rvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const 可变参数右值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for volatile variadic lvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief volatile 可变参数左值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) volatile &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for volatile variadic rvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief volatile 可变参数右值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) volatile &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  /// @~english
  /// @brief Specialization for const volatile variadic lvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const volatile 可变参数左值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const volatile &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };
  /// @~english
  /// @brief Specialization for const volatile variadic rvalue reference member functions
  /// @tparam C Class type
  /// @tparam R Return type
  /// @tparam Args Function argument types
  ///
  /// @~chinese
  /// @brief const volatile 可变参数右值引用成员函数特化
  /// @tparam C 类类型
  /// @tparam R 返回类型
  /// @tparam Args 函数参数类型
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const volatile &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  namespace detail {

    /// @~english
    /// @brief Concept to check if a type has a lambda name
    /// @tparam T Type to check
    ///
    /// @~chinese
    /// @brief 检查类型是否具有 lambda 名称的概念
    /// @tparam T 要检查的类型
    template<typename T>
    concept has_lambda_name = std::is_class_v<T> && type_of<T>().contains("<lambda");

  }  // namespace detail

  /// @~english
  /// @brief Specialization for callable objects (lambdas and functors)
  /// @tparam T Callable object type
  ///
  /// @~chinese
  /// @brief 可调用对象（lambda 和仿函数）的特化
  /// @tparam T 可调用对象类型
  template<typename T>
    requires callable<T> && std::is_class_v<T>
  struct function_traits<T> {
    using result_type = typename function_traits<decltype(&T::operator())>::result_type;
    using args_tuple = typename function_traits<decltype(&T::operator())>::args_tuple;
    using class_type = typename function_traits<decltype(&T::operator())>::class_type;
    static constexpr function_type type = detail::has_lambda_name<T> ? function_type::lambda :
                                                                       function_type::functor;
    static constexpr function_qualifiers qualifiers = detail::has_lambda_name<T> ?
                                                          (std::is_empty_v<T> ?
                                                               function_qualifiers::none :
                                                               function_qualifiers::has_capture) :
                                                          function_qualifiers::none;
  };

  // 辅助概念

  /// @~english
  /// @brief Concept for variadic functions
  /// @tparam T Function type to check
  ///
  /// @~chinese
  /// @brief 可变参数函数的概念
  /// @tparam T 要检查的函数类型
  template<typename T>
  concept variadic_function = has_qualifier(function_traits<T>::qualifiers,
                                            function_qualifiers::is_variadic);

  /// @~english
  /// @brief Concept for const functions
  /// @tparam T Function type to check
  ///
  /// @~chinese
  /// @brief const 函数的概念
  /// @tparam T 要检查的函数类型
  template<typename T>
  concept const_function = has_qualifier(function_traits<T>::qualifiers,
                                         function_qualifiers::is_const);

  /// @~english
  /// @brief Concept for volatile functions
  /// @tparam T Function type to check
  ///
  /// @~chinese
  /// @brief volatile 函数的概念
  /// @tparam T 要检查的函数类型
  template<typename T>
  concept volatile_function = has_qualifier(function_traits<T>::qualifiers,
                                            function_qualifiers::is_volatile);

  /// @~english
  /// @brief Concept for lvalue reference functions
  /// @tparam T Function type to check
  ///
  /// @~chinese
  /// @brief 左值引用函数的概念
  /// @tparam T 要检查的函数类型
  template<typename T>
  concept lvalue_function = has_qualifier(function_traits<T>::qualifiers,
                                          function_qualifiers::is_lvalue);

  /// @~english
  /// @brief Concept for rvalue reference functions
  /// @tparam T Function type to check
  ///
  /// @~chinese
  /// @brief 右值引用函数的概念
  /// @tparam T 要检查的函数类型
  template<typename T>
  concept rvalue_function = has_qualifier(function_traits<T>::qualifiers,
                                          function_qualifiers::is_rvalue);

  /// @~english
  /// @brief Concept for noexcept functions
  /// @tparam T Function type to check
  ///
  /// @~chinese
  /// @brief noexcept 函数的概念
  /// @tparam T 要检查的函数类型
  template<typename T>
  concept noexcept_function = has_qualifier(function_traits<T>::qualifiers,
                                            function_qualifiers::is_noexcept);

  /// @~english
  /// @brief Concept for member functions
  /// @tparam T Function type to check
  ///
  /// @~chinese
  /// @brief 成员函数的概念
  /// @tparam T 要检查的函数类型
  template<typename T>
  concept member_function = function_traits<T>::type == function_type::member_function;

  /// @~english
  /// @brief Concept for lambda functions
  /// @tparam T Function type to check
  ///
  /// @~chinese
  /// @brief lambda 函数的概念
  /// @tparam T 要检查的函数类型
  template<typename T>
  concept lambda_function = function_traits<T>::type == function_type::lambda;

  /// @~english
  /// @brief Concept for functors
  /// @tparam T Function type to check
  ///
  /// @~chinese
  /// @brief 仿函数的概念
  /// @tparam T 要检查的函数类型
  template<typename T>
  concept functor = function_traits<T>::type == function_type::functor;

  /// @~english
  /// @brief Concept for free functions
  /// @tparam T Function type to check
  ///
  /// @~chinese
  /// @brief 自由函数的概念
  /// @tparam T 要检查的函数类型
  template<typename T>
  concept free_function = function_traits<T>::type == function_type::free_function;

  /// @~english
  /// @brief Concept for functions without value returned
  /// @tparam T Function type to check
  ///
  /// @~chinese
  /// @brief 无返回值函数的概念
  /// @tparam T 要检查的函数类型
  template<typename T>
  concept irreturned_function = std::is_same_v<function_traits<T>::return_type, void>;

  /// @~english
  /// @brief Concept for functions with value returned
  /// @tparam T Function type to check
  ///
  /// @~chinese
  /// @brief 有返回值函数的概念
  /// @tparam T 要检查的函数类型
  template<typename T>
  concept returned_function = !std::is_same_v<function_traits<T>::return_type, void>;

}  // namespace traits
