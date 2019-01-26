/// \file meta.cpp Tests for Meta: a tiny metaprogramming library
// Meta: a tiny metaprogramming library
//
//  Copyright Eric Niebler 2013-2015
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Acknowledgements: Thanks for Paul Fultz for the suggestions that
//                   concepts can be ordinary Boolean metafunctions.
//
// Project home: https://github.com/ericniebler/range-v3
//

#include <meta/meta.hpp>
#include <tuple>
#include "simple_test.hpp"

using namespace meta;

#ifdef META_CONCEPT
static_assert(meta::Integral<std::true_type>, "");
static_assert(meta::Integral<std::false_type>, "");
static_assert(meta::Integral<std::integral_constant<int, 42>>, "");
static_assert(meta::Integral<meta::size_t<42>>, "");

template <class T, T Value>
struct not_integral_constant
{
    using type = not_integral_constant;
    using value_type = T;

    static const T value;
    constexpr operator T() const noexcept { return Value; }
    constexpr T operator()() const noexcept { return Value; }
};

static_assert(!meta::Integral<void>, "");
static_assert(!meta::Integral<int>, "");
static_assert(!meta::Integral<not_integral_constant<int, 42>>, "");
#endif

// An implementation of tuple_cat gives Range v3's meta-programming and list
// utilities a good workout. It's a good compiler stress test, too.

namespace tc_detail
{
    template <typename Ret, typename... Is, typename... Ks, typename Tuples>
    Ret tuple_cat_(list<Is...>, list<Ks...>, Tuples tpls)
    {
        return Ret{std::get<Ks::value>(std::get<Is::value>(tpls))...};
    }
} // namespace tc_detail

template <typename... Tuples, typename Res = apply<quote<std::tuple>, concat<as_list<Tuples>...>>>
Res tuple_cat(Tuples &&... tpls)
{
    static constexpr std::size_t N = sizeof...(Tuples);
    // E.g. [0,0,0,2,2,2,3,3]
    using inner =
        join<transform<list<as_list<Tuples>...>,
                       transform<as_list<make_index_sequence<N>>, quote<id>>, quote<transform>>>;
    // E.g. [0,1,2,0,1,2,0,1]
    using outer = join<
        transform<list<as_list<Tuples>...>,
                  compose<quote<as_list>, quote_i<std::size_t, make_index_sequence>, quote<size>>>>;
    return tc_detail::tuple_cat_<Res>(inner{}, outer{},
                                      std::forward_as_tuple(std::forward<Tuples>(tpls)...));
}

void test_tuple_cat()
{
    std::tuple<int, short, long> t1;
    std::tuple<> t2;
    std::tuple<float, double, long double> t3;
    std::tuple<void *, char *> t4;

    auto x = ::tuple_cat(t1, t2, t3, t4);
    using expected_type = std::tuple<int, short, long, float, double, long double, void *, char *>;
    static_assert(std::is_same<decltype(x), expected_type>::value, "");
}

// static tests for meta

namespace test_detail
{
    template <typename, typename, typename = void>
    struct can_invoke_ : std::false_type
    {
    };

    template <typename F, typename... As>
    struct can_invoke_<F, list<As...>, void_<invoke<F, As...>>> : std::true_type
    {
    };
} // namespace test_detail

template <typename F, typename... As>
using can_invoke = test_detail::can_invoke_<F, list<As...>>;

namespace test_can_invoke
{
    static_assert(can_invoke<meta::quote<std::pair>, int, int>::value, "");

#if defined(META_WORKAROUND_GCC_64970)
    static_assert(!can_invoke<meta::quote<std::pair>, int, int, int>::value, "");
#endif
} // namespace test_can_invoke

namespace test_meta_group
{
    namespace test_trait_group
    {
        // id
        static_assert(is_trait<id<int_<1>>>::value, "");
        static_assert(is_callable<id<int_<1>>>::value, "");
        static_assert(std::is_same<_t<id<int_<2>>>, int_<2>>::value, "");

        // id_t
        static_assert(is_trait<meta::id_t<int_<1>>>::value, "");
        static_assert(!is_callable<meta::id_t<int_<1>>>::value, "");
        static_assert(std::is_same<meta::id_t<int_<1>>, int_<1>>::value, "");
        static_assert(std::is_same<meta::id_t<int_<1>>, _t<id<int_<1>>>>::value, "");
        static_assert(std::is_same<meta::id_t<int_<1>>, _t<_t<lazy::id<int_<1>>>>>::value, "");

        // alignof_
        static_assert(!equal_to<alignof_<char>, alignof_<uint64_t>>::value, "");
        static_assert(equal_to<alignof_<unsigned long>, alignof_<long>>::value, "");

        // is
        static_assert(meta::is<std::tuple<int, double, char>, std::tuple>::value, "");
        static_assert(not_<meta::is<std::tuple<int, double, char>, list>>::value, "");

        // is_callable
        namespace detail
        {
            template <std::size_t i>
            using inc_c = meta::inc<meta::size_t<i>>;
        }
        static_assert(is_callable<quote<std::is_same>>::value, "");
        static_assert(!is_callable<std::is_same<protect<_a>, protect<_b>>>::value, "");
        static_assert(is_callable<quote_trait<std::is_const>>::value, "");
        static_assert(!is_callable<std::is_const<_a>>::value, "");
        static_assert(is_callable<quote_i<std::size_t, meta::make_index_sequence>>::value, "");
        static_assert(!is_callable<meta::make_index_sequence<10>>::value, "");
        static_assert(is_callable<quote_trait_i<std::size_t, detail::inc_c>>::value, "");
        static_assert(!is_callable<detail::inc_c<2>>::value, "");

        namespace detail
        {
            struct nested_t
            {
                using type = void;
            };

            struct empty_t
            {
            };
        } // namespace detail
        // is_trait
        static_assert(is_trait<detail::nested_t>::value, "");
        static_assert(!is_trait<detail::empty_t>::value, "");

        template <typename T>
        using has_nested_t = let<is_valid<lazy::_t<T>>>;
        static_assert(has_nested_t<detail::nested_t>::value, "");
        static_assert(!has_nested_t<detail::empty_t>::value, "");

        // sizeof_
        static_assert(meta::sizeof_<int>::value == sizeof(int), "");

        // let
        template <typename T, typename List>
        using find_index_ = let<var<_a, List>, var<_b, lazy::find<_a, T>>,
                                lazy::if_<std::is_same<_b, list<>>, meta::npos,
                                          lazy::minus<lazy::size<_a>, lazy::size<_b>>>>;
        static_assert(find_index_<int, list<short, int, float>>::value == 1, "");
        static_assert(equal_to<find_index_<double, list<short, int, float>>, meta::npos>::value,
                      "");

        // not_fn
        static_assert(invoke<not_fn<quote_trait<is_trait>>, detail::empty_t>::value, "");
        static_assert(invoke<not_fn<not_fn<quote_trait<is_trait>>>, detail::nested_t>::value, "");

        // sizeof_
        static_assert(equal_to<sizeof_<list<>>, meta::size_t<sizeof(list<>)>>::value, "");

        // void_
        namespace detail
        {
            template <typename, typename = void_<>>
            struct has_nested_type_alias_ : std::false_type
            {
            };

            template <typename T>
            struct has_nested_type_alias_<T, void_<typename T::type>> : std::true_type
            {
            };
        } // namespace detail
        static_assert(detail::has_nested_type_alias_<detail::nested_t>::value, "");

        // lambda
        namespace detail
        {
            template <typename A, int B = 0>
            struct lambda_test
            {
            };
        } // namespace detail

        using Lambda0 = lambda<_a, _b, std::pair<_a, _b>>;
        using Lambda1 = lambda<_a, _b, std::pair<_b, _a>>;
        using Lambda2 = lambda<_a, _b, std::pair<_b, std::pair<_a, _a>>>;
        using Pair0 = invoke<Lambda0, int, short>;
        using Pair1 = invoke<Lambda1, int, short>;
        using Pair2 = invoke<Lambda2, int, short>;
        static_assert(std::is_same<Pair0, std::pair<int, short>>::value, "");
        static_assert(std::is_same<Pair1, std::pair<short, int>>::value, "");
        static_assert(std::is_same<Pair2, std::pair<short, std::pair<int, int>>>::value, "");

        using X = invoke<lambda<_args, list<_args>>, int, short, double>;
        static_assert(std::is_same<X, list<int, short, double>>::value, "");

        using X2 = invoke<lambda<_a, detail::lambda_test<_a>>, int>;
        static_assert(std::is_same<X2, detail::lambda_test<_a>>::value, "");

        using Y = invoke<lambda<_args, defer<std::pair, _args>>, int, short>;
        static_assert(std::is_same<Y, std::pair<int, short>>::value, "");

        using Y2 = invoke<lambda<_args, list<_args, list<_args>>>, int, short>;
        static_assert(std::is_same<Y2, list<int, short, list<int, short>>>::value, "");

        using Z = invoke<lambda<_a, _args, list<int, _args, double, _a>>, short *, short, float>;
        static_assert(std::is_same<Z, list<int, short, float, double, short *>>::value, "");

        // Nesting variadic lambdas in non-variadic lambdas:
        using A = invoke<lambda<_a, lazy::invoke<lambda<_b, _args, list<_args, _b>>, _a,
                                                 lazy::_t<std::add_pointer<_a>>,
                                                 lazy::_t<std::add_lvalue_reference<_a>>>>,
                         int>;
        static_assert(std::is_same<A, list<int *, int &, int>>::value, "");

        // Nesting non-variadic lambdas in variadic lambdas:
        using B = invoke<lambda<_a, _args, lazy::invoke<lambda<_b, list<_b, _args, _a>>, _a>>, int,
                         short, double>;
        static_assert(std::is_same<B, list<int, short, double, int>>::value, "");

        // Nesting variadic lambdas in variadic lambdas:
        using ZZ = invoke<
            lambda<_a, _args_a,
                   lazy::invoke<lambda<_b, _args_b, list<_b, _a, list<_args_b>, list<_args_a>>>,
                                _args_a>>,
            int, short, float, double>;
        static_assert(
            std::is_same<ZZ,
                         list<short, int, list<float, double>, list<short, float, double>>>::value,
            "");

        // pathological lambda test
        using P = invoke<lambda<_a, detail::lambda_test<_a>>, int>;
        static_assert(std::is_same<P, detail::lambda_test<_a>>::value, "");

#if defined(META_WORKAROUND_GCC_64970)
        static_assert(!can_invoke<lambda<_args, defer<std::pair, _args>>, int>::value, "");
        static_assert(
            !can_invoke<lambda<_args, defer<std::pair, _args>>, int, short, double>::value, "");
        static_assert(!can_invoke<lambda<_a, defer<std::pair, _a, _a>>, int, short>::value, "");
        static_assert(!can_invoke<lambda<_a, _b, _c, _args, defer<std::pair, _a, _a>>>::value, "");
#endif

        namespace test_lazy_trait_group
        {
            namespace detail
            {
                struct nested_t
                {
                    using type = void;
                };

            } // namespace detail
            // lazy::alignof_
            static_assert(
                equal_to<invoke<lambda<_a, lazy::alignof_<_a>>, int>, alignof_<int>>::value, "");

            // lazy::sizeof_
            static_assert(equal_to<invoke<lambda<_a, lazy::sizeof_<_a>>, int>, sizeof_<int>>::value,
                          "");
            // lazy::not_fn
            static_assert(
                std::is_same<invoke<lambda<_a, lazy::not_fn<_a>>, quote<std::is_integral>>,
                             not_fn<quote<std::is_integral>>>::value,
                "");

            // lazy::let
            template <typename T>
            using lazy_has_nested_t = lazy::let<is_valid<lazy::_t<T>>>;
            static_assert(_t<lazy_has_nested_t<detail::nested_t>>::value, "");
        } // namespace test_lazy_trait_group

        namespace test_invocation_group
        {
            // _t
            static_assert(is_trait<int_<1>>::value, "");
            static_assert(std::is_same<_t<int_<1>>, typename int_<1>::type>::value, "");
            static_assert(std::is_same<_t<lazy::_t<int_<1>>>, typename int_<1>::type>::value, "");
            static_assert(std::is_same<_t<int_<1>>, int_<1>>::value, "");

            namespace test_lazy_invocation_group
            {
                // lazy::id
                static_assert(
                    std::is_same<_t<_t<lazy::id<int_<1>>>>, typename int_<1>::type>::value, "");
                static_assert(std::is_same<_t<_t<lazy::id<int_<1>>>>, int_<1>>::value, "");
                static_assert(
                    std::is_same<
                        _t<lazy::if_<defer<is_trait, int_<1>>, lazy::id<int_<1>>, lazy::_t<nil_>>>,
                        lazy::id<int_<1>>>::value,
                    "");

                // lazy::_t
                static_assert(
                    std::is_same<
                        _t<lazy::if_<defer<is_trait, int_<1>>, lazy::_t<int_<1>>, lazy::_t<nil_>>>,
                        lazy::_t<int_<1>>>::value,
                    "");
                static_assert(std::is_same<_t<lazy::_t<int_<1>>>, int_<1>>::value, "");

            } // namespace test_lazy_invocation_group

        } // namespace test_invocation_group

        namespace test_composition_group
        {
            // bind_front
            using is_float = bind_front<quote<std::is_same>, float>;
            static_assert(invoke<is_float, float>::value, "");
            static_assert(!invoke<is_float, double>::value, "");

            // bind_back
            using is_float2 = bind_back<quote<std::is_same>, float>;
            static_assert(invoke<is_float2, float>::value, "");
            static_assert(!invoke<is_float2, double>::value, "");

            // compose
            using composed_t = compose<quote_trait<std::add_lvalue_reference>,
                                       quote_trait<std::add_const>, quote_trait<std::make_signed>>;
            static_assert(std::is_same<invoke<composed_t, unsigned>, int const &>::value, "");

            // flip
            using unflipped_t = concat<list<int_<5>, int_<10>>, list<int_<2>>, list<int_<1>>>;
            static_assert(
                std::is_same<unflipped_t, list<int_<5>, int_<10>, int_<2>, int_<1>>>::value, "");
            static_assert(std::is_same<invoke<flip<quote<concat>>, list<int_<5>, int_<10>>,
                                              list<int_<2>>, list<int_<1>>>,
                                       list<int_<2>, int_<5>, int_<10>, int_<1>>>::value,
                          "");

            // quote, quote_trait, quote_i, quote_trait_i
            static_assert(invoke<quote<std::is_same>, int, int>::value, "");
            static_assert(invoke<quote_trait<std::is_const>, const int>::value, "");
            static_assert(
                std::is_same<invoke<quote_i<std::size_t, make_index_sequence>, meta::size_t<10>>,
                             make_index_sequence<10>>::value,
                "");
            static_assert(
                std::is_same<invoke<quote_trait_i<std::size_t, detail::inc_c>, meta::size_t<1>>,
                             meta::size_t<2>>::value,
                "");

            // curry, uncurry
            static_assert(std::is_same<invoke<uncurry<curry<quote_trait<id>>>,
                                              std::tuple<int, short, double>>,
                                       list<int, short, double>>::value,
                          "");

            // on
            static_assert(equal_to<invoke<on<quote<dec>, quote<negate>, quote<inc>>, int_<10>>,
                                   int_<-12>>::value,
                          "");
            namespace test_lazy_composition_group
            {
            }
        } // namespace test_composition_group

    } // namespace test_trait_group
} // namespace test_meta_group

namespace test_logical_group
{
    // Test that and_ gets short-circuited:
    template <typename T>
    using test_and_ = let<and_<std::is_void<T>, defer<std::is_convertible, T>>>;
    static_assert(std::is_same<test_and_<int>, std::false_type>::value, "");

    // Test that or_ gets short-circuited:
    template <typename T>
    using test_or_ = let<or_<std::is_void<T>, defer<std::is_convertible, T>>>;
    static_assert(std::is_same<test_or_<void>, std::true_type>::value, "");

    // if_
    static_assert(std::is_same<if_<std::is_integral<int>, int>, int>::value, "");
    static_assert(
        std::is_same<
            invoke<lambda<_a, lazy::_t<std::remove_cv<lazy::_t<std::remove_reference<_a>>>>>,
                   int const &>,
            int>::value,
        "");

    // Test that the unselected branch does not get evaluated:
    template <typename T>
    using test_if_ = let<if_<std::is_void<T>, T, defer<std::pair, T>>>;
    static_assert(std::is_same<test_if_<void>, void>::value, "");

#if defined(META_WORKAROUND_GCC_64970)
    static_assert(!can_invoke<lambda<_a, lazy::if_<std::is_integral<_a>, _a>>, float>::value, "");
#endif

    // lazy::if_c
    template <typename N>
    struct fact
      : let<lazy::if_c<(N::value > 0), lazy::multiplies<N, defer<fact, dec<N>>>, meta::size_t<1>>>
    {
    };

    static_assert(fact<meta::size_t<0>>::value == 1, "");
    static_assert(fact<meta::size_t<1>>::value == 1, "");
    static_assert(fact<meta::size_t<2>>::value == 2, "");
    static_assert(fact<meta::size_t<3>>::value == 6, "");
    static_assert(fact<meta::size_t<4>>::value == 24, "");

    template <std::size_t N>
    struct fact2
      : let<lazy::if_c<(N > 0),
                       lazy::multiplies<meta::size_t<N>, defer_i<std::size_t, fact2, N - 1>>,
                       meta::size_t<1>>>
    {
    };

    static_assert(fact2<0>::value == 1, "");
    static_assert(fact2<1>::value == 1, "");
    static_assert(fact2<2>::value == 2, "");
    static_assert(fact2<3>::value == 6, "");
    static_assert(fact2<4>::value == 24, "");

    template <typename N>
    struct factorial
      : let<if_c<N::value == 0, meta::size_t<1>, lazy::multiplies<N, factorial<lazy::dec<N>>>>>
    {
    };

    static_assert(factorial<meta::size_t<0>>::value == 1, "");
    static_assert(factorial<meta::size_t<1>>::value == 1, "");
    static_assert(factorial<meta::size_t<2>>::value == 2, "");
    static_assert(factorial<meta::size_t<3>>::value == 6, "");
    static_assert(factorial<meta::size_t<4>>::value == 24, "");
    namespace test_lazy_logical_group
    {
        // Test that and_ gets short-circuited:
        template <typename T>
        using test_lazy_and_ = let<lazy::and_<std::is_void<T>, defer<std::is_convertible, T>>>;
        static_assert(std::is_same<test_lazy_and_<int>, std::false_type>::value, "");

        // Test that or_ gets short-circuited:
        template <typename T>
        using test_lazy_or_ = let<lazy::or_<std::is_void<T>, defer<std::is_convertible, T>>>;
        static_assert(std::is_same<test_lazy_or_<void>, std::true_type>::value, "");

        // lazy::if_
        static_assert(can_invoke<lambda<_a, lazy::if_<std::is_integral<_a>, _a>>, int>::value, "");
        static_assert(
            std::is_same<
                invoke<lambda<_a, lazy::_t<std::remove_cv<lazy::_t<std::remove_reference<_a>>>>>,
                       int const &>,
                int>::value,
            "");

        // Test that the unselected branch does not get evaluated:
        template <typename T>
        using test_lazy_if_ = let<lazy::if_<std::is_void<T>, T, defer<std::pair, T>>>;
        static_assert(std::is_same<test_lazy_if_<void>, void>::value, "");
#if defined(META_WORKAROUND_GCC_64970)
        static_assert(!can_invoke<lambda<_a, lazy::if_<std::is_integral<_a>, _a>>, float>::value,
                      "");
#endif

        // lazy::if_c
        template <typename N>
        struct fact : let<lazy::if_c<(N::value > 0), lazy::multiplies<N, defer<fact, dec<N>>>,
                                     meta::size_t<1>>>
        {
        };

        static_assert(fact<meta::size_t<0>>::value == 1, "");
        static_assert(fact<meta::size_t<1>>::value == 1, "");
        static_assert(fact<meta::size_t<2>>::value == 2, "");
        static_assert(fact<meta::size_t<3>>::value == 6, "");
        static_assert(fact<meta::size_t<4>>::value == 24, "");

        template <std::size_t N>
        struct fact2
          : let<lazy::if_c<(N > 0),
                           lazy::multiplies<meta::size_t<N>, defer_i<std::size_t, fact2, N - 1>>,
                           meta::size_t<1>>>
        {
        };

        static_assert(fact2<0>::value == 1, "");
        static_assert(fact2<1>::value == 1, "");
        static_assert(fact2<2>::value == 2, "");
        static_assert(fact2<3>::value == 6, "");
        static_assert(fact2<4>::value == 24, "");

        template <typename N>
        struct factorial
          : let<if_c<N::value == 0, meta::size_t<1>, lazy::multiplies<N, factorial<lazy::dec<N>>>>>
        {
        };

        static_assert(factorial<meta::size_t<0>>::value == 1, "");
        static_assert(factorial<meta::size_t<1>>::value == 1, "");
        static_assert(factorial<meta::size_t<2>>::value == 2, "");
        static_assert(factorial<meta::size_t<3>>::value == 6, "");
        static_assert(factorial<meta::size_t<4>>::value == 24, "");
    } // namespace test_lazy_logical_group

} // namespace test_logical_group

namespace test_algorithm_group
{
    namespace test_query_group
    {
        // all_of
        static_assert(all_of<list<int, short, long>, quote_trait<std::is_integral>>::value, "");
        // none_of
        static_assert(none_of<list<int, short, long>, quote_trait<std::is_floating_point>>::value,
                      "");
        // any_of
        static_assert(!any_of<list<int, short, long>, quote_trait<std::is_floating_point>>::value,
                      "");
        static_assert(
            any_of<list<int, short, long, float>, quote_trait<std::is_floating_point>>::value, "");

        // in
        static_assert(in<list<int, int, short, float>, int>::value, "");
        static_assert(in<list<int, int, short, float>, short>::value, "");
        static_assert(in<list<int, int, short, float>, float>::value, "");
        static_assert(!in<list<int, int, short, float>, double>::value, "");

        // find, find_if, reverse_find, reverse_find_if
        using list_to_search = list<int, short, int, float>;
        static_assert(std::is_same<find<list_to_search, int>, list<int, short, int, float>>::value,
                      "");

        static_assert(std::is_same<find_if<list_to_search, bind_front<quote<std::is_same>, int>>,
                                   list<int, short, int, float>>::value,
                      "");
        static_assert(std::is_same<find_if<list_to_search, bind_front<quote<std::is_same>, double>>,
                                   list<>>::value,
                      "");
        static_assert(std::is_same<reverse_find<list_to_search, int>, list<int, float>>::value, "");
        static_assert(
            std::is_same<reverse_find_if<list_to_search, bind_front<quote<std::is_same>, int>>,
                         list<int, float>>::value,
            "");
        static_assert(
            std::is_same<reverse_find_if<list_to_search, bind_front<quote<std::is_same>, double>>,
                         list<>>::value,
            "");

        // meta::find_index
        using searchable_list = list<int, long, short, int>;
        static_assert(find_index<searchable_list, int>::value == 0, "");
        static_assert(find_index<searchable_list, long>::value == 1, "");
        static_assert(find_index<searchable_list, short>::value == 2, "");
        static_assert(equal_to<find_index<searchable_list, double>, meta::npos>::value, "");
        static_assert(equal_to<find_index<searchable_list, float>, meta::npos>::value, "");

        using empty_list = list<>;
        static_assert(equal_to<find_index<empty_list, double>, meta::npos>::value, "");

        using callable_find_index = lambda<_a, _b, lazy::find_index<_b, _a>>;
        using result = invoke<callable_find_index, long, searchable_list>;
        static_assert(result{} == 1, "");

        // reverse_find_index
        static_assert(reverse_find_index<searchable_list, int>::value == 3, "");
        static_assert(reverse_find_index<searchable_list, long>::value == 1, "");
        static_assert(reverse_find_index<searchable_list, short>::value == 2, "");
        static_assert(equal_to<reverse_find_index<searchable_list, double>, meta::npos>::value, "");
        static_assert(equal_to<reverse_find_index<searchable_list, float>, meta::npos>::value, "");

        using empty_list = meta::list<>;
        static_assert(equal_to<reverse_find_index<empty_list, double>, meta::npos>::value, "");

        using callable_reverse_find_index =
            meta::lambda<_a, _b, meta::lazy::reverse_find_index<_b, _a>>;
        using result = meta::invoke<callable_reverse_find_index, long, searchable_list>;
        static_assert(result{} == 1, "");

        // meta::count
        static_assert(meta::count<searchable_list, int>::value == 2, "");
        static_assert(meta::count<searchable_list, short>::value == 1, "");
        static_assert(meta::count<searchable_list, double>::value == 0, "");

        // meta::count_if
        static_assert(meta::count_if<searchable_list, lambda<_a, std::is_same<_a, int>>>::value ==
                          2,
                      "");
        static_assert(meta::count_if<searchable_list, lambda<_b, std::is_same<_b, short>>>::value ==
                          1,
                      "");
        static_assert(
            meta::count_if<searchable_list, lambda<_c, std::is_same<_c, double>>>::value == 0, "");

        namespace test_lazy_query_group
        {
            // lazy::all_of
            static_assert(
                _t<lazy::all_of<list<int, short, long>, quote_trait<std::is_integral>>>::value, "");
            // lazy::none_of
            static_assert(_t<lazy::none_of<list<int, short, long>,
                                           quote_trait<std::is_floating_point>>>::value,
                          "");
            // lazy::any_of
            static_assert(!_t<lazy::any_of<list<int, short, long>,
                                           quote_trait<std::is_floating_point>>>::value,
                          "");
            static_assert(_t<lazy::any_of<list<int, short, long, float>,
                                          quote<std::is_floating_point>>>::value,
                          "");

            // lazy::in
            static_assert(let<lazy::in<list<int, int, short, float>, int>>::value, "");
            static_assert(let<lazy::in<list<int, int, short, float>, short>>::value, "");
            static_assert(let<lazy::in<list<int, int, short, float>, float>>::value, "");
            static_assert(!let<lazy::in<list<int, int, short, float>, double>>::value, "");
        } // namespace test_lazy_query_group

    } // namespace test_query_group

    namespace test_transformation_group
    {
        // filter
        namespace detail
        {
            using mixed_unfiltered_list = meta::list<int, double, short, float, long, char>;
            using unfiltered_int_list = meta::list<int, short, long, char>;
            using unfilitered_fp_list = meta::list<double, float>;
        } // namespace detail

        static_assert(std::is_same<detail::unfiltered_int_list,
                                   meta::filter<detail::mixed_unfiltered_list,
                                                meta::quote<std::is_integral>>>::value,
                      "");
        static_assert(std::is_same<detail::unfilitered_fp_list,
                                   meta::filter<detail::mixed_unfiltered_list,
                                                meta::quote<std::is_floating_point>>>::value,
                      "");

        // unique
        static_assert(
            std::is_same<meta::unique<list<int, short, int, double, short, double, double>>,
                         list<int, short, double>>::value,
            "");

        // cartesian_product
        template <class L>
        using cart_prod = reverse_fold<
            L, list<list<>>,
            lambda<_a, _b,
                   lazy::join<lazy::transform<
                       _b, lambda<_c, lazy::join<lazy::transform<
                                          _a, lambda<_d, list<lazy::push_front<_d, _c>>>>>>>>>>;

        using CartProd = cart_prod<meta::list<meta::list<int, short>, meta::list<float, double>>>;
        static_assert(
            std::is_same<CartProd,
                         meta::list<meta::list<int, float>, meta::list<int, double>,
                                    meta::list<short, float>, meta::list<short, double>>>::value,
            "");
        static_assert(
            std::is_same<CartProd,
                         cartesian_product<list<list<int, short>, list<float, double>>>>::value,
            "");

        // reverse
        static_assert(
            std::is_same<reverse<list<int, short, double>>, list<double, short, int>>::value, "");
        static_assert(std::is_same<reverse<list<int, short, double, float>>,
                                   list<float, double, short, int>>::value,
                      "");
        static_assert(
            std::is_same<
                reverse<
                    list<int[1], int[2], int[3], int[4], int[5], int[6], int[7], int[8], int[9],
                         int[10], int[11], int[12], int[13], int[14], int[15], int[16], int[17]>>,
                list<int[17], int[16], int[15], int[14], int[13], int[12], int[11], int[10], int[9],
                     int[8], int[7], int[6], int[5], int[4], int[3], int[2], int[1]>>::value,
            "");

        // sort
        using unsorted_list = list<char[5], char[3], char[2], char[6], char[1], char[5], char[10]>;
        using sorted_list =
            meta::sort<unsorted_list,
                       lambda<_a, _b, lazy::less<lazy::sizeof_<_a>, lazy::sizeof_<_b>>>>;
        using expected_list = list<char[1], char[2], char[3], char[5], char[5], char[6], char[10]>;
        static_assert(std::is_same<sorted_list, expected_list>::value, "");

        // partition
        using unpartitioned_list =
            list<char[5], char[3], char[2], char[6], char[1], char[5], char[10]>;

        namespace detail
        {
            struct is_even
            {
                template <typename N>
                using invoke = bool_<N::type::value % 2 == 0>;
            };
            static_assert(!invoke<not_fn<quote<is_even::invoke>>, int_<2>>::value, "");

            template <typename L>
            using get_size_t = let<var<_a, L>, lazy::size<_a>>;

        } // namespace detail

        using int_range_1_10_list = as_list<meta::integer_range<int, 1, 11>>;
        static_assert(invoke<detail::is_even, int_<2>>::value, "");

        using even_numbers_first_list = partition<int_range_1_10_list, detail::is_even>;
        static_assert(std::is_same<even_numbers_first_list,
                                   list<list<int_<2>, int_<4>, int_<6>, int_<8>, int_<10>>,
                                        list<int_<1>, int_<3>, int_<5>, int_<7>, int_<9>>>>::value,
                      "");
        using char_arrays_list =
            list<char[5], char[3], char[2], char[6], char[1], char[5], char[10]>;
        using char_arrays_sz_gt_5_first_list =
            partition<char_arrays_list,
                      lambda<_a, lazy::greater<lazy::sizeof_<_a>, sizeof_<char[5]>>>>;

        static_assert(std::is_same<char_arrays_sz_gt_5_first_list,
                                   list<list<char[6], char[10]>,
                                        list<char[5], char[3], char[2], char[1], char[5]>>>::value,
                      "");

        // fold
        static_assert(meta::fold<meta::as_list<meta::integer_range<std::size_t, 0, 5>>,
                                 meta::size_t<0>, meta::quote<meta::plus>>::value == 10,
                      "");

        static_assert(meta::fold<meta::as_list<meta::integer_range<std::size_t, 5, 10>>,
                                 meta::size_t<0>, meta::quote<meta::plus>>::value == 35,
                      "");

        static_assert(equal_to<meta::fold<meta::as_list<meta::integer_range<std::size_t, 0, 20>>,
                                          meta::size_t<0>, meta::quote<meta::plus>>,
                               meta::size_t<190>>::value,
                      "");

        static_assert(std::is_same<meta::integer_range<std::size_t, 5, 10>,
                                   meta::integer_sequence<std::size_t, 5, 6, 7, 8, 9>>::value,
                      "");

        // reverse_fold
        static_assert(meta::reverse_fold<meta::as_list<meta::integer_range<std::size_t, 0, 5>>,
                                         meta::size_t<0>, meta::quote<meta::plus>>::value == 10,
                      "");

        static_assert(meta::reverse_fold<meta::as_list<meta::integer_range<std::size_t, 5, 10>>,
                                         meta::size_t<0>, meta::quote<meta::plus>>::value == 35,
                      "");

        static_assert(
            equal_to<meta::reverse_fold<meta::as_list<meta::integer_range<std::size_t, 0, 20>>,
                                        meta::size_t<0>, meta::quote<meta::plus>>,
                     meta::size_t<190>>::value,
            "");

        template <typename List>
        using rev = reverse_fold<List, list<>, lambda<_a, _b, defer<push_back, _a, _b>>>;
        static_assert(std::is_same<rev<list<int, short, double>>, list<double, short, int>>::value,
                      "");

        namespace test_lazy_transformation_group
        {
            // lazy::sort
            using unsorted_list =
                list<char[5], char[3], char[2], char[6], char[1], char[5], char[10]>;
            using lazy_sorted_list =
                lazy::sort<unsorted_list,
                           lambda<_a, _b, lazy::less<lazy::sizeof_<_a>, lazy::sizeof_<_b>>>>;
            using expected_list =
                list<char[1], char[2], char[3], char[5], char[5], char[6], char[10]>;
            static_assert(std::is_same<_t<lazy_sorted_list>, expected_list>::value, "");

            // lazy::partition
            using lazy_partition_gt_len_5 = let<
                lazy::partition<unpartitioned_list,
                                lambda<_a, lazy::greater<lazy::sizeof_<_a>, sizeof_<char[5]>>>>>;
            using lazy_partition_gt_len_1 = let<
                lazy::partition<unpartitioned_list,
                                lambda<_a, lazy::greater<lazy::sizeof_<_a>, sizeof_<char[1]>>>>>;
            static_assert(detail::get_size_t<lazy_partition_gt_len_5>{} == 2, "");
            static_assert(
                std::is_same<lazy_partition_gt_len_5,
                             list<list<char[6], char[10]>,
                                  list<char[5], char[3], char[2], char[1], char[5]>>>::value,
                "");
            static_assert(detail::get_size_t<lazy_partition_gt_len_1>{} == 2, "");
            static_assert(
                std::is_same<lazy_partition_gt_len_1,
                             list<list<char[5], char[3], char[2], char[6], char[5], char[10]>,
                                  list<char[1]>>>::value,
                "");
        } // namespace test_lazy_transformation_group

    } // namespace test_transformation_group

    namespace test_runtime_group
    {
        namespace detail
        {
            struct check_integral
            {
                template <class T>
                constexpr T operator()(T &&i) const
                {
                    static_assert(std::is_integral<T>::value, "");
                    return i;
                }
            };

        } // namespace detail
        using runtime_list = meta::list<int, long, short>;
        constexpr auto r = meta::for_each(runtime_list{}, detail::check_integral());
        static_assert(
            std::is_same<meta::_t<std::remove_cv<decltype(r)>>, detail::check_integral>::value, "");
    } // namespace test_runtime_group

} // namespace test_algorithm_group

namespace test_datatype_group
{
    // nil_ has no nested type
    static_assert(!is_trait<nil_>::value, "");

    namespace detail
    {
        struct t1
        {
        };
        struct t2
        {
        };

        struct t1_f final
        {
            // inherited types cannot be final
        };
    } // namespace detail

    using t3 = inherit<list<detail::t1, detail::t2>>;
    static_assert(std::is_base_of<detail::t1, t3>::value, "");
    static_assert(std::is_base_of<detail::t2, t3>::value, "");
    // list of inherited types must be unique
    static_assert(std::is_same<inherit<unique<list<detail::t1, detail::t1>>>,
                               inherit<list<detail::t1>>>::value,
                  "");

#if defined(META_WORKAROUND_GCC_64970)
    static_assert(!can_invoke<quote<inherit>, detail::t1, detail::t1>::value, "");
#endif

#if defined(META_WORKAROUND_GCC_64970)
    static_assert(!can_invoke<quote<inherit>, detail::t1_f>::value, "");
#endif

    namespace test_lazy_datatype_group
    {
        // lazy::inherit
        using t3_lazy = lazy::inherit<list<detail::t1, detail::t2>>;
        static_assert(!std::is_base_of<detail::t1, t3_lazy>::value, "");
        static_assert(!std::is_base_of<detail::t2, t3_lazy>::value, "");

        static_assert(std::is_base_of<detail::t1, _t<t3_lazy>>::value, "");
        static_assert(std::is_base_of<detail::t2, _t<t3_lazy>>::value, "");
    } // namespace test_lazy_datatype_group

    namespace test_list_group
    {
        // list
        static_assert(!std::is_same<list<int, char, void>, std::tuple<int, char, void>>::value, "");

        // as_list
        static_assert(
            std::is_same<list<int, char, void>, as_list<std::tuple<int, char, void>>>::value, "");
        static_assert(std::is_same<as_list<list<int, char, void>>,
                                   as_list<std::tuple<int, char, void>>>::value,
                      "");

        // list::size()
        static_assert(list<int, char, void>::size() == size<list<int, char, void>>::value, "");

        // at_c
        static_assert(std::is_same<at_c<list<int, char, void>, 2>, void>::value, "");

        // at
        static_assert(std::is_same<at<list<int, char, void>, meta::size_t<0>>, int>::value, "");

#if defined(META_WORKAROUND_GCC_64970)
        static_assert(!can_invoke<quote<at>, list<int, char, void>, meta::size_t<3>>::value, "");
#endif
        namespace test_lazy_list_group
        {
        }
    } // namespace test_list_group

    namespace test_integral_group
    {
#if META_CXX_VER >= META_CXX_STD_14
        static_assert(std::is_same<std::integer_sequence<int, 0, 1, 2>,
                                   meta::make_integer_sequence<int, 3>>::value,
                      "");
        static_assert(std::is_same<std::integer_sequence<std::size_t, 0, 1, 2>,
                                   meta::make_index_sequence<3>>::value,
                      "");
        static_assert(std::is_same<std::integer_sequence<int, 1, 3, 2>,
                                   meta::integer_sequence<int, 1, 3, 2>>::value,
                      "");
        static_assert(std::is_same<std::integer_sequence<std::size_t, 1, 3, 2>,
                                   meta::index_sequence<1, 3, 2>>::value,
                      "");
#endif

#if META_CXX_INTEGER_SEQUENCE
        static_assert(std::is_same<as_list<meta::make_index_sequence<3>>,
                                   list<meta::size_t<0>, meta::size_t<1>, meta::size_t<2>>>::value,
                      "");
        static_assert(std::is_same<as_list<meta::make_integer_sequence<int, 3>>,
                                   list<int_<0>, int_<1>, int_<2>>>::value,
                      "");
#endif

        static_assert(equal_to<std::integral_constant<bool, true>, bool_<true>>::value, "");
        static_assert(equal_to<std::integral_constant<bool, false>, bool_<false>>::value, "");
        static_assert(equal_to<std::integral_constant<char, 'a'>, char_<'a'>>::value, "");
        static_assert(equal_to<std::integral_constant<int, 10>, int_<10>>::value, "");
        static_assert(equal_to<std::integral_constant<std::size_t, 10>, meta::size_t<10>>::value,
                      "");
        static_assert(equal_to<std::integral_constant<int, 10>, int_<10>>::value, "");

        // operator""_z
        static_assert(42_z == 42, "");
    } // namespace test_integral_group

    namespace test_extension_group
    {
    }

} // namespace test_datatype_group

namespace test_math_group
{
    // inc
    static_assert(equal_to<std::integral_constant<int, 2>, inc<int_<1>>>::value, "");
    // dec
    static_assert(equal_to<std::integral_constant<int, 1>, dec<int_<2>>>::value, "");
    // plus
    static_assert(equal_to<std::integral_constant<int, 3>, plus<int_<2>, int_<1>>>::value, "");
    static_assert(equal_to<plus<int_<1>, int_<2>>, plus<int_<2>, int_<1>>>::value, "");

    // minus
    static_assert(equal_to<std::integral_constant<int, 1>, minus<int_<3>, int_<2>>>::value, "");
    static_assert(!equal_to<minus<int_<1>, int_<2>>, minus<int_<2>, int_<1>>>::value, "");

    // multiplies
    static_assert(equal_to<std::integral_constant<int, 6>, multiplies<int_<3>, int_<2>>>::value,
                  "");
    static_assert(equal_to<multiplies<int_<3>, int_<2>>, multiplies<int_<2>, int_<3>>>::value, "");

    // divides
    static_assert(equal_to<std::integral_constant<int, 1>, divides<int_<3>, int_<2>>>::value, "");
    static_assert(!equal_to<divides<int_<1>, int_<2>>, divides<int_<2>, int_<1>>>::value, "");

    // negate
    static_assert(equal_to<int_<-1>, negate<int_<1>>>::value, "");

    // modulus
    static_assert(equal_to<modulus<int_<10>, int_<2>>, int_<0>>::value, "");
    static_assert(equal_to<modulus<int_<11>, int_<2>>, int_<1>>::value, "");

    // greater
    static_assert(greater<int_<11>, int_<10>>::value, "");
    static_assert(!greater<int_<11>, int_<11>>::value, "");
    static_assert(!greater<int_<11>, int_<12>>::value, "");

    // greater_equal
    static_assert(greater_equal<int_<11>, int_<10>>::value, "");
    static_assert(greater_equal<int_<11>, int_<11>>::value, "");
    static_assert(!greater_equal<int_<11>, int_<12>>::value, "");

    // less
    static_assert(!less<int_<11>, int_<10>>::value, "");
    static_assert(!less<int_<11>, int_<11>>::value, "");
    static_assert(less<int_<11>, int_<12>>::value, "");

    // less_equal
    static_assert(!less_equal<int_<11>, int_<10>>::value, "");
    static_assert(less_equal<int_<11>, int_<11>>::value, "");
    static_assert(less_equal<int_<11>, int_<12>>::value, "");

    // bit_and
    static_assert(equal_to<bit_and<int_<10>, int_<15>>, int_<10>>::value, "");
    static_assert(equal_to<bit_and<int_<1>, int_<2>>, int_<0>>::value, "");

    // bit_or
    static_assert(equal_to<bit_or<int_<10>, int_<15>>, int_<15>>::value, "");
    static_assert(equal_to<bit_or<int_<1>, int_<2>>, int_<3>>::value, "");

    // bit_xor
    static_assert(equal_to<bit_xor<int_<1>, int_<1>>, int_<0>>::value, "");
    static_assert(equal_to<bit_xor<int_<10>, int_<15>>, int_<5>>::value, "");

    // bit_not
    static_assert(equal_to<bit_not<int_<15>>, int_<-16>>::value, "");
    static_assert(equal_to<bit_not<int_<0>>, int_<-1>>::value, "");

    // min
    static_assert(meta::min<meta::size_t<0>, meta::size_t<1>>::value == 0, "");
    static_assert(meta::min<meta::size_t<0>, meta::size_t<0>>::value == 0, "");
    static_assert(meta::min<meta::size_t<1>, meta::size_t<0>>::value == 0, "");

    // max
    static_assert(meta::max<meta::size_t<0>, meta::size_t<1>>::value == 1, "");
    static_assert(meta::max<meta::size_t<1>, meta::size_t<0>>::value == 1, "");
    static_assert(meta::max<meta::size_t<1>, meta::size_t<1>>::value == 1, "");

    namespace test_lazy_math_group
    {
        // lazy::inc
        static_assert(std::is_same<invoke<lambda<_a, lazy::inc<_a>>, int_<1>>, int_<2>>::value, "");

        // lazy::dec
        static_assert(std::is_same<invoke<lambda<_a, lazy::dec<_a>>, int_<2>>, int_<1>>::value, "");

        // lazy::plus
        static_assert(std::is_same<invoke<lambda<_a, _b, lazy::plus<_a, _b>>, int_<3>, int_<2>>,
                                   int_<5>>::value,
                      "");

        // lazy::minus
        static_assert(invoke<lambda<_a, _b, _c, lazy::equal_to<_a, lazy::minus<_b, _c>>>, int_<1>,
                             int_<3>, int_<2>>::value,
                      "");

        // lazy::equal_to
        static_assert(invoke<lambda<_a, _b, _c, lazy::equal_to<_a, lazy::multiplies<_b, _c>>>,
                             int_<2>, int_<2>, int_<1>>::value,
                      "");
        static_assert(invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>, int_<6>,
                             lazy::multiplies<int_<3>, int_<2>>>::value,
                      "");

        // lazy::multiplies
        static_assert(
            invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>, lazy::multiplies<int_<3>, int_<2>>,
                   lazy::multiplies<int_<2>, int_<3>>>::value,
            "");

        // lazy::divides
        static_assert(invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>, int_<1>,
                             lazy::divides<int_<3>, int_<2>>>::value,
                      "");
        static_assert(
            !invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>, lazy::divides<int_<3>, int_<2>>,
                    lazy::multiplies<int_<2>, int_<3>>>::value,
            "");

        // lazy::negate
        static_assert(
            invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>, int_<-1>, lazy::negate<int_<1>>>::value,
            "");

        // lazy::modulus
        static_assert(invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>,
                             lazy::modulus<int_<10>, int_<2>>, int_<0>>::value,
                      "");
        static_assert(invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>,
                             lazy::modulus<int_<11>, int_<2>>, int_<1>>::value,
                      "");

        // lazy::greater
        static_assert(invoke<lambda<_a, _b, lazy::greater<_a, _b>>, int_<11>, int_<10>>::value, "");
        static_assert(!invoke<lambda<_a, _b, lazy::greater<_a, _b>>, int_<11>, int_<11>>::value,
                      "");
        static_assert(!invoke<lambda<_a, _b, lazy::greater<_a, _b>>, int_<11>, int_<12>>::value,
                      "");

        // lazy::greater_equal
        static_assert(
            invoke<lambda<_a, _b, lazy::greater_equal<_a, _b>>, int_<11>, int_<10>>::value, "");
        static_assert(
            invoke<lambda<_a, _b, lazy::greater_equal<_a, _b>>, int_<11>, int_<11>>::value, "");
        static_assert(
            !invoke<lambda<_a, _b, lazy::greater_equal<_a, _b>>, int_<11>, int_<12>>::value, "");

        // lazy::less
        static_assert(!invoke<lambda<_a, _b, lazy::less<_a, _b>>, int_<11>, int_<10>>::value, "");
        static_assert(!invoke<lambda<_a, _b, lazy::less<_a, _b>>, int_<11>, int_<11>>::value, "");
        static_assert(invoke<lambda<_a, _b, lazy::less<_a, _b>>, int_<11>, int_<12>>::value, "");

        // lazy::less_equal
        static_assert(!invoke<lambda<_a, _b, lazy::less_equal<_a, _b>>, int_<11>, int_<10>>::value,
                      "");
        static_assert(invoke<lambda<_a, _b, lazy::less_equal<_a, _b>>, int_<11>, int_<11>>::value,
                      "");
        static_assert(invoke<lambda<_a, _b, lazy::less_equal<_a, _b>>, int_<11>, int_<12>>::value,
                      "");

        // lazy::bit_and
        static_assert(invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>,
                             lazy::bit_and<int_<10>, int_<15>>, int_<10>>::value,
                      "");
        static_assert(invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>,
                             lazy::bit_and<int_<1>, int_<2>>, int_<0>>::value,
                      "");

        // lazy::bit_or
        static_assert(invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>,
                             lazy::bit_or<int_<10>, int_<15>>, int_<15>>::value,
                      "");
        static_assert(invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>, lazy::bit_or<int_<1>, int_<2>>,
                             int_<3>>::value,
                      "");

        // lazy::bit_xor
        static_assert(invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>,
                             lazy::bit_xor<int_<1>, int_<1>>, int_<0>>::value,
                      "");
        static_assert(invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>,
                             lazy::bit_xor<int_<10>, int_<15>>, int_<5>>::value,
                      "");

        // lazy::bit_not
        static_assert(invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>, lazy::bit_not<int_<15>>,
                             int_<-16>>::value,
                      "");
        static_assert(
            invoke<lambda<_a, _b, lazy::equal_to<_a, _b>>, lazy::bit_not<int_<0>>, int_<-1>>::value,
            "");
    } // namespace test_lazy_math_group

} // namespace test_math_group

template <typename T>
struct undef_t;

int main()
{
    test_tuple_cat();
    return ::test_result();
}
