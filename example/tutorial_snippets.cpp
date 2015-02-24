/// \file metafunction.cpp Code snippets for the tutorial
///
/// Meta: a tiny metaprogramming library
///
///  Copyright Eric Niebler 2013-2015
///
///  Use, modification and distribution is subject to the
///  Boost Software License, Version 1.0. (See accompanying
///  file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// Project home: https://github.com/ericniebler/meta
///
#include <tuple>
#include <type_traits>
#include <meta/meta.hpp>

template <class T>
struct dump;

namespace metafunction0
{
    /// [meta_function0]
    template <typename... Args>
    struct mf
    {
        using type = void;
    };
    using result = typename mf<int, double>::type;
    static_assert(std::is_same<result, void>{}, "");
    /// [meta_function0]
}

using metafunction0::mf;

namespace metafunction1
{
    /// [meta_function1]
    template <typename... Args>
    using mf_t = typename mf<Args...>::type;
    using result = mf_t<int, double>;
    static_assert(std::is_same<result, void>{}, "");
    /// [meta_function1]
}

using metafunction1::mf_t;

namespace metafunction2
{
    /// [meta_function2]
    template <typename... Args>
    using mf2_t = meta::eval<mf<Args...>>;
    using result = mf2_t<int, double>;
    static_assert(std::is_same<result, void>{}, "");
    /// [meta_function2]
}

namespace metafunction_class0
{
    /// [meta_function_class0]
    struct mfc
    {
        template <typename... Args>
        using apply = void;
    };
    /// [meta_function_class0]

    /// [meta_function_class1]
    using result = mfc::apply<int, double>;
    static_assert(std::is_same<result, void>{}, "");
    /// [meta_function_class1]
}

namespace metafunction_class1
{
    /// [meta_function_class2]
    using mf_class0 = meta::quote<mf>;
    using result0 = meta::apply<mf_class0, int, double>;
    static_assert(std::is_same<result0, mf<int, double>>{}, "");
    static_assert(std::is_same<meta::eval<result0>, void>{}, "");

    using mf_class1 = meta::quote<mf_t>;
    using result1 = meta::apply<mf_class1, int, double>;
    static_assert(std::is_same<result1, void>{}, "");
    /// [meta_function_class2]
}

namespace type_list0
{
    /// [type_list0]
    using list0 = meta::list<int, double>;
    using list1 = meta::list<>;
    using list2 = meta::list<float>;
    using result = meta::concat<list0, list1, list2>;
    static_assert(std::is_same<result, meta::list<int, double, float>>{}, "");
    /// [type_list0]
}

namespace type_list1
{
    /// [type_list1]
    using list0 = meta::list<int, double>;
    using list1 = meta::list<>;
    using list2 = meta::list<float>;
    using list_of_lists = meta::list<list0, list1, list2>;
    using result = meta::join<list_of_lists>;
    static_assert(std::is_same<result, meta::list<int, double, float>>{}, "");
    /// [type_list1]
}

namespace type_list2
{
    /// [type_list2]
    using t = std::tuple<int, double, float>;
    using l = meta::as_list<t>;
    static_assert(std::is_same<l, meta::list<int, double, float>>{}, "");

    using i = meta::make_index_sequence<3>;
    using il = meta::as_list<i>;
    static_assert(std::is_same<il, meta::list<std::integral_constant<std::size_t, 0>,
                                              std::integral_constant<std::size_t, 1>,
                                              std::integral_constant<std::size_t, 2>>>{},
                  "");
    /// [type_list2]
}

namespace composition0
{
    /// [composition0]
    template <class T>
    using mf0 = meta::eval<std::make_signed<T>>;
    template <class T>
    using mf1 = meta::eval<std::add_const<T>>;
    template <class T>
    using mf2 = meta::eval<std::add_lvalue_reference<T>>;

    using mf = meta::compose<meta::quote<mf2>, meta::quote<mf1>, meta::quote<mf0>>;
    static_assert(std::is_same<meta::apply<mf, unsigned>, int const &>{}, "");
    /// [composition0]
}

int main()
{
    return 0;
}
