/// \file trait.cpp Code snippets for the tutorial
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

namespace trait0
{
    /// [trait0]
    template <typename... Args>
    struct t
    {
        using type = void;
    };
    using result = typename t<int, double>::type;
    static_assert(std::is_same<result, void>{}, "");
    /// [trait0]
}

using trait0::t;

namespace trait1
{
    /// [trait1]
    template <typename... Args>
    using t_t = typename t<Args...>::type;
    using result = t_t<int, double>;
    static_assert(std::is_same<result, void>{}, "");
    /// [trait1]
}

using trait1::t_t;

namespace trait2
{
    /// [trait2]
    template <typename... Args>
    using t2_t = meta::_t<t<Args...>>;
    using result = t2_t<int, double>;
    static_assert(std::is_same<result, void>{}, "");
    /// [trait2]
}

namespace callable0
{
    /// [callable0]
    struct ac
    {
        template <typename... Args>
        using invoke = void;
    };
    /// [callable0]

    /// [callable1]
    using result = meta::invoke<ac, int, double>;
    static_assert(std::is_same<result, void>{}, "");
    /// [callable1]
}

namespace callable1
{
    /// [callable2]
    using t_callable0 = meta::quote<t>;
    using result0 = meta::invoke<t_callable0, int, double>;
    static_assert(std::is_same<result0, t<int, double>>{}, "");
    static_assert(std::is_same<meta::_t<result0>, void>{}, "");

    using t_callable1 = meta::quote<t_t>;
    using result1 = meta::invoke<t_callable1, int, double>;
    static_assert(std::is_same<result1, void>{}, "");
    /// [callable2]
}

namespace callable2
{
    /// [callable3]
    using t_callable0 = meta::quote_trait<std::add_pointer>;
    using result0 = meta::invoke<t_callable0, int>;
    static_assert(std::is_same<result0, int *>{}, "");

#if __cplusplus > 201103L
    using t_callable1 = meta::quote<std::add_pointer_t>;
    using result1 = meta::invoke<t_callable1, int>;
    static_assert(std::is_same<result1, int *>{}, "");
#endif
    /// [callable3]
}

namespace partial_application0
{
    /// [partial_application0]
    using is_float = meta::bind_front<meta::quote<std::is_same>, float>;
    static_assert(meta::invoke<is_float, float>{}, "");
    static_assert(!meta::invoke<is_float, double>{}, "");

    using is_float2 = meta::bind_back<meta::quote<std::is_same>, float>;
    static_assert(meta::invoke<is_float2, float>{}, "");
    static_assert(!meta::invoke<is_float2, double>{}, "");
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

    static_assert(meta::invoke<greater, meta::size_t<2>, meta::size_t<1>>{}, "");
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

    using at_1 = meta::at_c<list, 1>;

    static_assert(std::is_same<at_1, double>{}, "");

    using index_double = meta::find_index<list, double>;
    static_assert(index_double{} == 1, "");

    using index_char = meta::find_index<list, char>;
    static_assert(index_char{} == meta::npos(), "");

    static_assert(!meta::empty<list>{}, "");
    /// [type_list0]
}

namespace type_list1
{
    using list = type_list0::list;
    /// [type_list1]
    using i = meta::size_t<1>;
    using at_1 = meta::at<list, i>;
    static_assert(std::is_same<at_1, double>{}, "");
    /// [type_list1]
}

namespace type_list2
{
    using list = type_list0::list;
    /// [type_list2]
    using l2 = meta::push_front<list, char>;
    static_assert(std::is_same<l2, meta::list<char, int, double, float>>{}, "");

    using l3 = meta::pop_front<l2>; // equivalent to meta::drop<l2, 1>;
    static_assert(std::is_same<l3, list>{}, "");

    using l4 = meta::push_back<list, char>;
    static_assert(std::is_same<l4, meta::list<int, double, float, char>>{}, "");

    using l5 = meta::drop_c<l4, 3>;
    static_assert(std::is_same<l5, meta::list<char>>{}, "");
    /// [type_list2]
} // namespace type_list2

namespace type_list3
{
    /// [type_list3]
    using list0 = meta::list<int, double>;
    using list1 = meta::list<>;
    using list2 = meta::list<float, char>;

    using concatenated = meta::concat<list0, list1, list2>;
    static_assert(std::is_same<concatenated, meta::list<int, double, float, char>>{}, "");

    using list_of_lists = meta::list<list0, list1, list2>;
    using flattened = meta::join<list_of_lists>;
    static_assert(std::is_same<flattened, meta::list<int, double, float, char>>{}, "");

    using list_of_lists_of_same_length = meta::list<list0, list2>;
    using zipped = meta::zip<list_of_lists_of_same_length>;
    static_assert(
        std::is_same<zipped, meta::list<meta::list<int, float>, meta::list<double, char>>>{}, "");
    /// [type_list3]
}

namespace type_list4
{
    using namespace meta::placeholders;

    /// [type_list4]
    using namespace meta::lazy;
    using l = meta::list<char, int, long, long long, float, float>;

    using size_of_largest_type =
        meta::fold<l, meta::size_t<0>, meta::lambda<_a, _b, max<_a, sizeof_<_b>>>>;
    static_assert(size_of_largest_type{} == meta::sizeof_<long long>{}, "");

    using largest_type =
        meta::fold<l, meta::nil_,
                   meta::lambda<_a, _b, if_<greater<sizeof_<_a>, sizeof_<_b>>, _a, _b>>>;
    static_assert(std::is_same<largest_type, long long>{}, "");

    using first_type_larger_than_char =
        meta::front<meta::find_if<l, meta::lambda<_a, greater<sizeof_<_a>, sizeof_<char>>>>>;
    static_assert(std::is_same<first_type_larger_than_char, int>{}, "");

    using unique_types = meta::unique<l>;
    static_assert(std::is_same<unique_types, meta::list<char, int, long, long long, float>>{}, "");
    /// [type_list4]
} // namespace type_list4

namespace type_list5
{
    /// [type_list5]
    using t = std::tuple<int, double, float>;
    using l = meta::as_list<t>;
    static_assert(std::is_same<l, meta::list<int, double, float>>{}, "");

    using i = meta::make_index_sequence<3>;
    using il = meta::as_list<i>;
    static_assert(std::is_same<il, meta::list<std::integral_constant<std::size_t, 0>,
                                              std::integral_constant<std::size_t, 1>,
                                              std::integral_constant<std::size_t, 2>>>{},
                  "");
    /// [type_list5]
}

namespace composition0
{
    /// [composition0]
    template <class T>
    using t0 = meta::_t<std::make_signed<T>>;
    template <class T>
    using t1 = meta::_t<std::add_const<T>>;
    template <class T>
    using t2 = meta::_t<std::add_lvalue_reference<T>>;

    using t = meta::compose<meta::quote<t2>, meta::quote<t1>, meta::quote<t0>>;
    static_assert(std::is_same<meta::invoke<t, unsigned>, int const &>{}, "");
    /// [composition0]
}

int main()
{
    return 0;
}
