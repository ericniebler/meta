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

namespace partial_application0
{
    /// [partial_application0]
    using is_float = meta::bind_front<meta::quote<std::is_same>, float>;
    static_assert(meta::apply<is_float, float>{}, "");
    static_assert(!meta::apply<is_float, double>{}, "");

    using is_float2 = meta::bind_back<meta::quote<std::is_same>, float>;
    static_assert(meta::apply<is_float2, float>{}, "");
    static_assert(!meta::apply<is_float2, double>{}, "");
    /// [partial_application0]
} // namespace partial_application0

namespace logical_operations0
{
    /// [logical_operations0]
    using t0 = meta::if_<std::is_same<float, double>, meta::bool_<true>, meta::bool_<false>>;
    static_assert(!t0{}, "");

    using t1 = meta::and_<meta::bool_<true>, meta::bool_<false>, meta::bool_<true>>;
    static_assert(!t1{}, "");

    using t2 = meta::or_<meta::bool_<true>, meta::bool_<false>, meta::bool_<true>>;
    static_assert(t2{}, "");

    using t3 = meta::not_<t1>;
    static_assert(t3{}, "");
    /// [logical_operations0]
} // namespace logical_operations0

namespace lambda0
{
    /// [lambda0]
    using namespace meta::placeholders;
    using greater = meta::lambda<_a, _b, meta::lazy::less<_b, _a>>;

    static_assert(meta::apply<greater, meta::size_t<2>, meta::size_t<1>>{}, "");
    /// [lambda0]
}

namespace type_list0
{
    /// [type_list0]
    using list = meta::list<int, double, float>;
    static_assert(list::size() == 3, "");

    using front = meta::front<list>;
    static_assert(std::is_same<front, int>{}, "");

    using back = meta::back<list>;
    static_assert(std::is_same<back, float>{}, "");

    using at_1 = meta::list_element_c<1, list>;

    static_assert(std::is_same<at_1, double>{}, "");

    static_assert(!meta::empty<list>{}, "");
    /// [type_list0]
}

namespace type_list1
{
    using list = type_list0::list;
    /// [type_list1]
    using i = meta::size_t<1>;
    using at_1 = meta::list_element<i, list>;
    static_assert(std::is_same<at_1, double>{}, "");
    /// [type_list1]
}

namespace type_list2
{
    using list = type_list0::list;
    /// [type_list2]
    using l2 = meta::push_front<list, char>;
    static_assert(std::is_same<l2, meta::list<char, int, double, float>>{}, "");

    using l3 = meta::pop_front<l2>; // equivalent to meta::drop<l2>;
    static_assert(std::is_same<l3, list>{}, "");

    using l4 = meta::push_back<list, char>;
    static_assert(std::is_same<l4, meta::list<int, double, float, char>>{}, "");

    using l5 = meta::drop_c<3, l4>;
    static_assert(std::is_same<l5, meta::list<char>>{}, "");
    /// [type_list2]
} // namespace type_list2

namespace type_list3
{
    /// [type_list3]
    using list0 = meta::list<int, double>;
    using list1 = meta::list<>;
    using list2 = meta::list<float>;
    using result = meta::concat<list0, list1, list2>;
    static_assert(std::is_same<result, meta::list<int, double, float>>{}, "");
    /// [type_list3]
}

namespace type_list4
{
    /// [type_list4]
    using list0 = meta::list<int, double>;
    using list1 = meta::list<>;
    using list2 = meta::list<float>;
    using list_of_lists = meta::list<list0, list1, list2>;
    using result = meta::join<list_of_lists>;
    static_assert(std::is_same<result, meta::list<int, double, float>>{}, "");
    /// [type_list4]
}

namespace type_list5
{
    /// [type_list5]
    using list0 = meta::list<int, short, unsigned>;
    using list1 = meta::list<float, double, char>;
    using result = meta::zip<meta::list<list0, list1>>;
    static_assert(std::is_same<result, meta::list<meta::list<int, float>, meta::list<short, double>,
                                                  meta::list<unsigned, char>>>{},
                  "");
    /// [type_list5]
} // namespace type_list5

namespace type_list6
{
    using namespace meta::placeholders;

    /// [type_list6]
    using namespace meta::lazy;
    using l = meta::list<short, int, long, long long, float, float>;

    using size_of_largest_type =
        meta::foldl<l, meta::size_t<0>, meta::lambda<_a, _b, max<_a, sizeof_<_b>>>>;
    static_assert(size_of_largest_type{} == meta::sizeof_<long long>{}, "");

    using largest_type =
        meta::foldl<l, meta::nil_,
                    meta::lambda<_a, _b, if_<greater<sizeof_<_a>, sizeof_<_b>>, _a, _b>>>;
    static_assert(std::is_same<largest_type, long long>{}, "");

    using first_type_larger_than_int =
        meta::front<meta::find_if<l, meta::lambda<_a, greater<sizeof_<_a>, sizeof_<int>>>>>;
    static_assert(std::is_same<first_type_larger_than_int, long>{}, "");

    using unique_types = meta::unique<l>;
    static_assert(std::is_same<unique_types, meta::list<short, int, long, long long, float>>{}, "");
    /// [type_list6]
} // namespace type_list6

namespace type_list7
{
    /// [type_list7]
    using t = std::tuple<int, double, float>;
    using l = meta::as_list<t>;
    static_assert(std::is_same<l, meta::list<int, double, float>>{}, "");

    using i = meta::make_index_sequence<3>;
    using il = meta::as_list<i>;
    static_assert(std::is_same<il, meta::list<std::integral_constant<std::size_t, 0>,
                                              std::integral_constant<std::size_t, 1>,
                                              std::integral_constant<std::size_t, 2>>>{},
                  "");
    /// [type_list7]
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
