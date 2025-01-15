//#pragma once
//
//#include <cassert>
//#include <functional>
//#include <iostream>
//#include <memory>
//#include <type_traits>
//
//#include "signal.h"
//
//namespace test {
//
//void free_function() {}
//int free_function_with_param(int a) {}
//
//struct Test {
//  void member_function() {}
//  void operator()() {}
//};
//
//struct ComplexFunctor {
//  int operator()(int x)
//  {
//    return x * 2;
//  }
//};
//
//using FunctionPointer = void (*)();
//using MemberFunctionPointer = void (Test::*)();
//using ComplexMemberFunctionPointer = int (ComplexFunctor::*)(int);
//
//static_assert(!traits::function_traits<int>::is_disconnectable,
//              "int should not be disconnectable");
//static_assert(traits::function_traits<FunctionPointer>::is_disconnectable,
//              "FunctionPointer should be disconnectable");
//static_assert(traits::function_traits<MemberFunctionPointer>::is_disconnectable,
//              "MemberFunctionPointer should be disconnectable");
//
//static_assert(traits::function_traits<Test>::is_disconnectable,
//              "Test (function object) should be disconnectable");
//static_assert(traits::function_traits<ComplexFunctor>::is_disconnectable,
//              "ComplexFunctor should be disconnectable");
//
//static_assert(traits::function_traits<FunctionPointer>::must_check_object == false,
//              "FunctionPointer must not require object check");
//static_assert(traits::function_traits<MemberFunctionPointer>::must_check_object == true,
//              "MemberFunctionPointer must require object check");
//
//static_assert(traits::max_size_of_function_ptr > 0,
//              "max_size_of_function_ptr should be greater than 0");
//static_assert(traits::max_size_of_function_ptr == sizeof(void *),
//              "max_size_of_function_ptr should match the size of a function pointer");
//
//static_assert(traits::weak_ptr<std::weak_ptr<int>>,
//              "std::weak_ptr<int> should satisfy weak_ptr concept");
//static_assert(!traits::weak_ptr<int>, "int should not satisfy weak_ptr concept");
//static_assert(traits::shared_ptr<std::shared_ptr<int>>,
//              "std::shared_ptr<int> should satisfy shared_ptr concept");
//static_assert(!traits::shared_ptr<int>, "int should not satisfy shared_ptr concept");
//
//static_assert(traits::callable<FunctionPointer>, "FunctionPointer should satisfy callable");
//static_assert(!traits::callable<int>, "int should not satisfy callable");
//static_assert(traits::member_callable<MemberFunctionPointer, Test>,
//              "MemberFunctionPointer should satisfy member_callable");
//static_assert(!traits::member_callable<MemberFunctionPointer, int>,
//              "int should not satisfy member_callable");
//
//constexpr bool test_function_ptr_storage()
//{
//  function_ptr fptr;
//  traits::function_traits<FunctionPointer>::set_ptr(free_function, fptr);
//  return traits::function_traits<FunctionPointer>::is_same_ptr(free_function, fptr);
//}
//static_assert(test_function_ptr_storage(),
//              "function_ptr should correctly store and compare free_function");
//
//constexpr bool test_member_function_ptr_storage()
//{
//  Test obj;
//  function_ptr fptr;
//  traits::function_traits<MemberFunctionPointer>::set_ptr(&Test::member_function, fptr);
//  return traits::function_traits<MemberFunctionPointer>::is_same_ptr(&Test::member_function, fptr);
//}
//static_assert(test_member_function_ptr_storage(),
//              "function_ptr should correctly store and compare member_function");
//
//constexpr bool test_function_object_storage()
//{
//  Test obj;
//  function_ptr fptr;
//  traits::function_traits<Test>::set_ptr(obj, fptr);
//  return traits::function_traits<Test>::is_same_ptr(obj, fptr);
//}
//static_assert(test_function_object_storage(),
//              "function_ptr should correctly store and compare function object");
//
//constexpr bool test_complex_functor_storage()
//{
//  ComplexFunctor obj;
//  function_ptr fptr;
//  traits::function_traits<ComplexFunctor>::set_ptr(obj, fptr);
//  return traits::function_traits<ComplexFunctor>::is_same_ptr(obj, fptr);
//}
//static_assert(test_complex_functor_storage(),
//              "function_ptr should correctly store and compare complex functor");
//
//}  // namespace test
