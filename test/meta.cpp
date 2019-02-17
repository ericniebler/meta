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

#include <limits>
#include <tuple>

#include <meta/meta.hpp>
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
    struct can_invoke_<F, list<As...>, void_<meta::invoke<F, As...>>> : std::true_type
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

namespace logical_sfinae_friendly
{
    // Regression test for #47
    //
    // Verify that and_, or_, strict_and, strict_or, and not_ are
    // SFINAE-friendly and do not produce a hard error when an evaluated type
    // does not model Integral.

    template <typename... Ts>
    meta::and_<Ts...> f0(int);

    template <typename... Ts>
    void f0(long);

    static_assert(std::is_same<void, decltype(f0<double>(42))>::value, "");

    template <typename... Ts>
    meta::or_<Ts...> f1(int);

    template <typename... Ts>
    void f1(long);

    static_assert(std::is_same<void, decltype(f1<double>(42))>::value, "");

    template <typename... Ts>
    meta::strict_and<Ts...> f2(int);

    template <typename... Ts>
    void f2(long);

    static_assert(std::is_same<void, decltype(f2<double>(42))>::value, "");

    template <typename... Ts>
    meta::strict_or<Ts...> f3(int);

    template <typename... Ts>
    void f3(long);

    static_assert(std::is_same<void, decltype(f3<double>(42))>::value, "");

    template <typename T>
    meta::not_<T> f4(int);

    template <typename T>
    void f4(long);

    static_assert(std::is_same<void, decltype(f4<double>(42))>::value, "");
}

namespace test_meta_group
{
    namespace test_trait_group
    {
        inline namespace
        {
            struct nested_t
            {
                using type = void;
            };

            struct empty_t
            {
            };

            static_assert(can_invoke<quote<_t>, nested_t>::value, "");
#if defined(META_WORKAROUND_GCC_64970)
            static_assert(!can_invoke<quote<_t>, empty_t>::value, "");
#endif

            template <std::size_t i>
            using inc_c = meta::inc<meta::size_t<i>>;
        } // namespace

        /**
         * \sa `meta::id_t`
         */
        static_assert(is_trait<meta::id_t<int_<1>>>::value, "");
        static_assert(!is_callable<meta::id_t<int_<1>>>::value, "");
        static_assert(std::is_same<int_<1>, meta::id_t<int_<1>>>::value, "");
        static_assert(std::is_same<_t<id<int_<1>>>, meta::id_t<int_<1>>>::value, "");
        static_assert(std::is_same<_t<_t<id<int_<1>>>>, meta::id_t<int_<1>>>::value, "");

        /**
         * \sa `meta::alignof_`
         */
        static_assert(!equal_to<alignof_<char>, alignof_<uint64_t>>::value, "");
        static_assert(equal_to<alignof_<unsigned long>, alignof_<long>>::value, "");

        /**
         * \sa `meta::is`
         */
        static_assert(meta::is<std::tuple<int, double, char>, std::tuple>::value, "");
        static_assert(not_<meta::is<std::tuple<int, double, char>, list>>::value, "");

        /**
         * \sa `meta::is_callable`
         */
        static_assert(is_callable<quote<std::is_same>>::value, "");
        static_assert(!is_callable<std::is_same<protect<_a>, protect<_b>>>::value, "");
        static_assert(is_callable<quote_trait<std::is_const>>::value, "");
        static_assert(!is_callable<std::is_const<_a>>::value, "");
        static_assert(is_callable<quote_i<std::size_t, meta::make_index_sequence>>::value, "");
        static_assert(!is_callable<meta::make_index_sequence<10>>::value, "");
        static_assert(is_callable<quote_trait_i<std::size_t, inc_c>>::value, "");
        static_assert(!is_callable<inc_c<2>>::value, "");

        /**
         * \sa `meta::is_trait`
         */
        static_assert(is_trait<nested_t>::value, "");
        static_assert(!is_trait<empty_t>::value, "");

        /**
         * \sa `meta::sizeof_`
         */
        static_assert(equal_to<sizeof_<list<>>, meta::size_t<sizeof(list<>)>>::value, "");
        static_assert(sizeof_<int>::value == sizeof(int), "");
        static_assert(sizeof_<list<int, char>>::value != list<int, char>::size(), "");

        /**
         * \sa `meta::let`
         */
        static_assert(std::is_same<int_<11>, let<lazy::inc<int_<10>>>>::value, "");
        static_assert(!is_callable<let<lazy::inc<int_<10>>>>::value, "");
        static_assert(is_callable<let<id<int_<10>>>>::value, "");

        /**
         * \brief Alternative implementation of `meta::find_index` using `meta::let`
         */
        template <typename T, typename List>
        using find_index_ = let<var<_a, List>, var<_b, lazy::find<_a, T>>,
                                lazy::if_<std::is_same<_b, list<>>, meta::npos,
                                          lazy::minus<lazy::size<_a>, lazy::size<_b>>>>;
        static_assert(
            std::is_same<meta::size_t<1>, find_index_<int, list<short, int, float>>>::value, "");
        static_assert(std::is_same<meta::npos, find_index_<double, list<short, int, float>>>::value,
                      "");

        /**
         * \sa `meta::not_fn`
         */
        static_assert(meta::invoke<not_fn<quote_trait<is_trait>>, empty_t>::value, "");
        static_assert(meta::invoke<not_fn<not_fn<quote_trait<is_trait>>>, nested_t>::value, "");

        /**
         * \sa `meta::void_`
         */
        namespace detail
        {
            template <typename, typename = void>
            struct has_nested_type_alias_ : std::false_type
            {
            };

            template <typename T>
            struct has_nested_type_alias_<T, void_<typename T::type>> : std::true_type
            {
            };
        } // namespace detail
        static_assert(detail::has_nested_type_alias_<nested_t>::value, "");
        static_assert(
            std::is_same<void_<int, double, detail::has_nested_type_alias_<float>>, void>::value,
            "");

        /**
         * \sa `meta::lambda`
         */
        namespace detail
        {
            template <typename A, int B = 0>
            struct lambda_test
            {
            };
        } // namespace detail

        /**
         * \sa `meta::lambda`
         * \brief `meta::lambda` to generate `std::pair<int, short>`
         */
        static_assert(std::is_same<meta::invoke<lambda<_a, _b, std::pair<_a, _b>>, int, short>,
                                   std::pair<int, short>>::value,
                      "");

        /**
         * \sa `meta::lambda`
         * \brief `meta::lambda` to generate `std::pair<short, int>`
         */
        static_assert(std::is_same<meta::invoke<lambda<_a, _b, std::pair<_b, _a>>, int, short>,
                                   std::pair<short, int>>::value,
                      "");
        /**
         * \sa `meta::lambda`
         * \brief `meta::lambda` to generate `std::pair<int, int>`
         */
        static_assert(
            std::is_same<meta::invoke<lambda<_a, _b, std::pair<_b, std::pair<_a, _a>>>, int, short>,
                         std::pair<short, std::pair<int, int>>>::value,
            "");

        /**
         * \sa `meta::lambda`
         * \brief `meta::lambda` using variadic arguments `meta::placeholders::_args`
         */
        static_assert(std::is_same<meta::invoke<lambda<_args, list<_args>>, int, short, double>,
                                   list<int, short, double>>::value,
                      "");

        /**
         * \sa `meta::lambda`
         * \brief \sa `meta::lambda` using \sa `meta::placeholders::_a`
         */
        static_assert(std::is_same<meta::invoke<lambda<_a, detail::lambda_test<_a>>, int>,
                                   detail::lambda_test<_a>>::value,
                      "");

        /**
         * \sa `meta::lambda`
         * \brief `meta::lambda` generating `std::pair<int, short>`
         */
        static_assert(std::is_same<meta::invoke<lambda<_args, defer<std::pair, _args>>, int, short>,
                                   std::pair<int, short>>::value,
                      "");

        /**
         * \sa `meta::lambda`
         * \brief `meta::lambda` generating `list<int, short, list<int, short>>`
         */
        static_assert(
            std::is_same<meta::invoke<lambda<_args, list<_args, list<_args>>>, int, short>,
                         list<int, short, list<int, short>>>::value,
            "");

        /**
         * \sa `meta::lambda`
         * \brief `meta::lambda` generating `list<int, short, float, double, short *>`
         */
        static_assert(std::is_same<meta::invoke<lambda<_a, _args, list<int, _args, double, _a>>,
                                                short *, short, float>,
                                   list<int, short, float, double, short *>>::value,
                      "");

        /**
         * \sa `meta::lambda`
         * \brief Nesting variadic `meta::lambda` types within non-variadic `meta::lambda` types
         */
        static_assert(
            std::is_same<
                meta::invoke<lambda<_a, lazy::invoke<lambda<_b, _args, list<_args, _b>>, _a,
                                                     lazy::_t<std::add_pointer<_a>>,
                                                     lazy::_t<std::add_lvalue_reference<_a>>>>,
                             int>,
                list<int *, int &, int>>::value,
            "");

        /**
         * \sa `meta::lambda`
         * \brief Nesting non-variadic `meta::lambda` types within variadic `meta::lambda` types
         */
        using B = meta::invoke<lambda<_a, _args, lazy::invoke<lambda<_b, list<_b, _args, _a>>, _a>>,
                               int, short, double>;
        static_assert(std::is_same<B, list<int, short, double, int>>::value, "");

        /**
         * \sa `meta::lambda`
         * \brief Nesting variadic `meta::lambda` types within variadic `meta::lambda` types
         */
        static_assert(
            std::is_same<
                meta::invoke<lambda<_a, _args_a,
                                    lazy::invoke<lambda<_b, _args_b,
                                                        list<_b, _a, list<_args_b>, list<_args_a>>>,
                                                 _args_a>>,
                             int, short, float, double>,
                list<short, int, list<float, double>, list<short, float, double>>>::value,
            "");

        /**
         * \sa `meta::lambda`
         * \brief Pathalogical `meta::lambda` test
         */
        static_assert(std::is_same<meta::invoke<lambda<_a, detail::lambda_test<_a>>, int>,
                                   detail::lambda_test<_a>>::value,
                      "");

#if defined(META_WORKAROUND_GCC_64970)
        static_assert(!can_invoke<quote<std::pair>, int>::value, "");
        static_assert(!can_invoke<quote<std::pair>, int, short, double>::value, "");
        static_assert(!can_invoke<lambda<_a, defer<std::pair, _a, _a>>, int, short>::value, "");
        static_assert(!can_invoke<lambda<_a, _b, _c, _args, defer<std::pair, _a, _a>>>::value, "");
#endif

        namespace test_lazy_trait_group
        {
            /**
             * \sa `meta::lazy::alignof_`
             */
            static_assert(std::is_same<alignof_<int>, let<lazy::alignof_<int>>>::value, "");

            /**
             * \sa `meta::lazy::sizeof_`
             */
            static_assert(std::is_same<sizeof_<int>, let<lazy::sizeof_<int>>>::value, "");

            /**
             * \sa `meta::lazy::not_fn`
             */
            static_assert(std::is_same<let<lazy::not_fn<quote<std::is_integral>>>,
                                       not_fn<quote<std::is_integral>>>::value,
                          "");

            /**
             * \sa lazy::let`
             */
            static_assert(let<is_valid<lazy::_t<nested_t>>>::value, "");
        } // namespace test_lazy_trait_group

        namespace test_invocation_group
        {
            /**
             * \sa `meta::defer`
             */
            static_assert(std::is_same<let<defer<id, int>>, id<int>>::value, "");

            /**
             * \sa `meta::defer_i`
             */
            static_assert(std::is_same<let<defer_i<std::size_t, inc_c, 1>>, meta::size_t<2>>::value,
                          "");

            /**
             * \sa `meta::id`
             */
            static_assert(is_trait<id<int_<1>>>::value, "");
            static_assert(is_callable<id<int_<1>>>::value, "");
            static_assert(std::is_same<int_<2>, _t<id<int_<2>>>>::value, "");

            /**
             * \sa `meta::_t
             */
            static_assert(is_trait<int_<1>>::value, "");
            static_assert(std::is_same<_t<int_<1>>, typename int_<1>::type>::value, "");
            static_assert(std::is_same<_t<int_<1>>, int_<1>>::value, "");

            /**
             * \sa `meta::apply`
             */
            static_assert(std::is_same<meta::apply<quote<std::is_same>, list<int, float>>,
                                       std::is_same<int, float>>::value,
                          "");

            /**
             * \sa `meta::invoke`
             */
            static_assert(std::is_same<meta::invoke<quote<std::is_same>, int, float>,
                                       std::is_same<int, float>>::value,
                          "");

#if META_CXX_VARIABLE_TEMPLATES
            /**
             * \sa `meta::_v`
             */
            static_assert(_v<std::is_same<meta::invoke<quote<std::is_same>, int, float>,
                                          std::is_same<int, float>>>,
                          "");
#endif
            namespace test_lazy_invocation_group
            {
                /**
                 * \sa `meta::lazy::id`
                 */
                static_assert(
                    std::is_same<_t<_t<lazy::id<int_<1>>>>, typename int_<1>::type>::value, "");
                static_assert(std::is_same<_t<_t<lazy::id<int_<1>>>>, int_<1>>::value, "");
                static_assert(std::is_same<_t<lazy::if_<is_trait<int_<1>>, lazy::id<int_<1>>,
                                                        lazy::invoke<quote<_t>, nil_>>>,
                                           lazy::id<int_<1>>>::value,
                              "");

                /**
                 * \sa `meta::lazy::_t`
                 */
                static_assert(std::is_same<let<lazy::_t<int_<1>>>, typename int_<1>::type>::value,
                              "");
                static_assert(std::is_same<_t<lazy::if_<is_trait<int_<1>>, lazy::_t<int_<1>>,
                                                        lazy::invoke<quote<_t>, nil_>>>,
                                           lazy::_t<int_<1>>>::value,
                              "");
                static_assert(std::is_same<_t<lazy::_t<int_<1>>>, int_<1>>::value, "");

                /**
                 * \sa `meta::lazy::invoke`
                 */
                static_assert(std::is_same<let<lazy::invoke<quote<std::is_same>, int, float>>,
                                           std::is_same<int, float>>::value,
                              "");

            } // namespace test_lazy_invocation_group

        } // namespace test_invocation_group

        namespace test_composition_group
        {
            /**
             * \sa `meta::bind_front`
             */
            using is_float = bind_front<quote<std::is_same>, float>;
            static_assert(meta::invoke<is_float, float>::value, "");
            static_assert(!meta::invoke<is_float, double>::value, "");

            /**
             * \sa `meta::bind_back`
             */
            using is_float2 = bind_back<quote<std::is_same>, float>;
            static_assert(meta::invoke<is_float2, float>::value, "");
            static_assert(!meta::invoke<is_float2, double>::value, "");

            /**
             * \sa `meta::compose`
             */
            using composed_t = compose<quote_trait<std::add_lvalue_reference>,
                                       quote_trait<std::add_const>, quote_trait<std::make_signed>>;
            static_assert(std::is_same<meta::invoke<composed_t, unsigned>, int const &>::value, "");

            /**
             * \sa `meta::flip`
             */
            using unflipped_t = concat<list<int_<5>, int_<10>>, list<int_<2>>, list<int_<1>>>;
            static_assert(
                std::is_same<unflipped_t, list<int_<5>, int_<10>, int_<2>, int_<1>>>::value, "");
            static_assert(std::is_same<meta::invoke<flip<quote<concat>>, list<int_<5>, int_<10>>,
                                                    list<int_<2>>, list<int_<1>>>,
                                       list<int_<2>, int_<5>, int_<10>, int_<1>>>::value,
                          "");

            /**
             * \sa `meta::quote`
             */
            static_assert(meta::invoke<quote<std::is_same>, int, int>::value, "");

            /**
             * \sa `meta::quote_trait`
             */
            static_assert(meta::invoke<quote_trait<std::is_const>, const int>::value, "");

            /**
             * \sa `meta::quote_i`
             */
            static_assert(std::is_same<meta::invoke<quote_i<std::size_t, make_index_sequence>,
                                                    meta::size_t<10>>,
                                       make_index_sequence<10>>::value,
                          "");
            /**
             * \sa `meta::quote_trait_i`
             */
            static_assert(
                std::is_same<meta::invoke<quote_trait_i<std::size_t, inc_c>, meta::size_t<1>>,
                             meta::size_t<2>>::value,
                "");

            /**
             * \sa `meta::curry`
             */
            static_assert(
                std::is_same<meta::invoke<curry<quote_trait<id>>, std::tuple<int, short, double>>,
                             list<std::tuple<int, short, double>>>::value,
                "");

            /**
             * \sa `meta::uncurry`
             */
            static_assert(std::is_same<meta::invoke<uncurry<curry<quote_trait<id>>>,
                                                    std::tuple<int, short, double>>,
                                       list<int, short, double>>::value,
                          "");

            /**
             * \sa `meta::on`
             */
            static_assert(
                equal_to<meta::invoke<on<quote<dec>, quote<negate>, quote<inc>>, int_<10>>,
                         int_<-12>>::value,
                "");
            namespace test_lazy_composition_group
            {
                /**
                 * \sa `meta::lazy::bind_front`
                 */
                template <typename T>
                using is_same_t = let<lazy::bind_front<quote<std::is_same>, T>>;
                static_assert(meta::invoke<is_same_t<float>, float>::value, "");
                static_assert(!meta::invoke<is_same_t<float>, double>::value, "");

                /**
                 * \sa `meta::lazy::bind_back`
                 */
                template <typename T>
                using is_same_t2 = let<lazy::bind_back<quote<std::is_same>, T>>;
                static_assert(meta::invoke<is_same_t2<float>, float>::value, "");
                static_assert(!meta::invoke<is_same_t2<float>, double>::value, "");

                /**
                 * \sa `meta::lazy::compose`
                 */
                static_assert(
                    std::is_same<std::add_const<std::add_lvalue_reference<int>>,
                                 invoke<let<lazy::compose<quote<std::add_const>,
                                                          quote<std::add_lvalue_reference>>>,
                                        int>>::value,
                    "");

                /**
                 * \sa `meta::lazy::flip`
                 */
                static_assert(
                    std::is_same<unflipped_t, list<int_<5>, int_<10>, int_<2>, int_<1>>>::value,
                    "");
                static_assert(
                    std::is_same<list<int_<2>, int_<5>, int_<10>, int_<1>>,
                                 invoke<let<lazy::flip<quote<concat>>>, list<int_<5>, int_<10>>,
                                        list<int_<2>>, list<int_<1>>>>::value,
                    "");

                /**
                 * \sa `meta::lazy::on`
                 */
                static_assert(
                    std::is_same<
                        int_<-12>,
                        invoke<let<lazy::on<quote<dec>, lazy::compose<quote<negate>, quote<inc>>>>,
                               int_<10>>>::value,
                    "");

                /**
                 * \sa `meta::lazy::curry`
                 */
                static_assert(std::is_same<list<std::tuple<int, short, double>>,
                                           invoke<let<lazy::curry<quote_trait<id>>>,
                                                  std::tuple<int, short, double>>>::value,
                              "");

                /**
                 * \sa `meta::lazy::uncurry`
                 */
                static_assert(std::is_same<list<int, short, double>,
                                           invoke<let<lazy::uncurry<curry<quote_trait<id>>>>,
                                                  std::tuple<int, short, double>>>::value,
                              "");
            } // namespace test_lazy_composition_group

        } // namespace test_composition_group

    } // namespace test_trait_group

    namespace test_logical_group
    {
        /**
         * \sa `meta::and_`
         */
        template <typename T>
        using test_and_ = let<and_<std::true_type, std::is_void<T>, defer<std::is_convertible, T>>>;
        static_assert(!test_and_<int>::value, "");
        static_assert(!and_<std::true_type, std::true_type, std::false_type>::value, "");
        static_assert(and_<std::true_type, std::true_type, std::true_type>::value, "");

        /**
         * \sa `meta::and_c`
         */
        template <typename T>
        using test_and_c = let<and_c<std::is_void<T>::value, true>>;
        static_assert(test_and_c<void>::value, "");
        static_assert(!and_c<true, true, false>::value, "");
        static_assert(and_c<true, true, true>::value, "");

        /**
         * \sa `meta::strict_and`
         */
        template <typename... Ts>
        using test_strict_and_ = strict_and<bool_<static_cast<bool>(Ts::type::value)>...>;
#if defined(META_WORKAROUND_GCC_64970)
        static_assert(!test_strict_and_<std::true_type, std::false_type,
                                        not_<can_invoke<quote<std::pair>, int, int, int>>>::value,
                      "");
#endif
        static_assert(!test_strict_and_<std::true_type, std::true_type, std::false_type>::value,
                      "");
        static_assert(test_strict_and_<std::true_type, std::true_type, std::true_type>::value, "");
        /**
         * \sa `meta::or_`
         */
        template <typename T>
        using test_or_ = let<or_<std::false_type, std::is_void<T>, defer<std::is_convertible, T>>>;
        static_assert(std::is_same<test_or_<void>, std::true_type>::value, "");
        static_assert(or_<std::true_type, std::true_type, std::false_type>::value, "");
        static_assert(!or_<std::false_type, std::false_type, std::false_type>::value, "");
        /**
         * \sa `meta::or_c`
         */
        template <typename T>
        using test_or_c = let<or_c<std::is_void<T>::value, true>>;
        static_assert(test_or_c<int>::value, "");
        static_assert(or_c<true, true, false>::value, "");
        static_assert(!or_c<false, false, false>::value, "");
        /**
         * \sa `meta::strict_or`
         */
        template <typename... Ts>
        using test_strict_or_ = strict_or<bool_<static_cast<bool>(Ts::type::value)>...>;
#if defined(META_WORKAROUND_GCC_64970)
        static_assert(test_strict_or_<std::true_type, std::false_type,
                                      not_<can_invoke<quote<std::pair>, int, int, int>>>::value,
                      "");
#endif
        static_assert(test_strict_or_<std::true_type, std::true_type, std::false_type>::value, "");
        static_assert(!test_strict_or_<std::false_type, std::false_type, std::false_type>::value,
                      "");

        /**
         * \sa `meta::if_`
         */
        static_assert(std::is_same<if_<std::is_integral<int>, int>, int>::value, "");

        /**
         * \brief Test `meta::if_` to verify unselected branch is not evaluated, even if ill-formed
         */
        template <typename T>
        using test_if_ = let<if_<std::is_void<T>, T, defer<std::pair, T>>>;
        static_assert(std::is_same<test_if_<void>, void>::value, "");

#if defined(META_WORKAROUND_GCC_64970)
        static_assert(!can_invoke<lambda<_a, lazy::if_<std::is_integral<_a>, _a>, float>>::value,
                      "");
#endif

        /**
         * \sa `meta::if_c`
         */

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

        /**
         * \sa `meta::not_`
         */
        static_assert(not_<std::is_void<int>>::value, "");

        /**
         * \sa `meta::not_c`
         */
        static_assert(not_c<std::is_void<int>::value>::value, "");

        namespace test_lazy_logical_group
        {
            /**
             * \sa `meta::lazy::and_`
             */
            template <typename T>
            using test_lazy_and_ =
                let<lazy::and_<std::true_type, std::is_void<T>, defer<std::is_convertible, T>>>;
            static_assert(std::is_same<test_lazy_and_<int>, std::false_type>::value, "");
            static_assert(!let<lazy::and_<std::true_type, std::true_type, std::false_type>>::value,
                          "");
            static_assert(let<lazy::and_<std::true_type, std::true_type, std::true_type>>::value,
                          "");

            /**
             * \sa `meta::lazy::strict_and`
             */
            template <typename... Ts>
            using test_lazy_strict_and_ =
                let<lazy::strict_and<bool_<static_cast<bool>(Ts::type::value)>...>>;
#if defined(META_WORKAROUND_GCC_64970)
            static_assert(
                !test_lazy_strict_and_<std::true_type, std::false_type,
                                       not_<can_invoke<quote<std::pair>, int, int, int>>>::value,
                "");
#endif
            static_assert(
                !test_lazy_strict_and_<std::true_type, std::true_type, std::false_type>::value, "");
            static_assert(
                test_lazy_strict_and_<std::true_type, std::true_type, std::true_type>::value, "");

            /**
             * \sa `meta::lazy::or_`
             */
            template <typename T>
            using test_lazy_or_ =
                let<lazy::or_<std::false_type, std::is_void<T>, defer<std::is_convertible, T>>>;
            static_assert(std::is_same<test_or_<void>, std::true_type>::value, "");
            static_assert(let<lazy::or_<std::true_type, std::true_type, std::false_type>>::value,
                          "");
            static_assert(!let<lazy::or_<std::false_type, std::false_type, std::false_type>>::value,
                          "");

            /**
             * \sa `meta::lazy::strict_or`
             */

            template <typename... Ts>
            using test_lazy_strict_or_ =
                let<lazy::strict_or<bool_<static_cast<bool>(Ts::type::value)>...>>;
#if defined(META_WORKAROUND_GCC_64970)

            static_assert(
                test_lazy_strict_or_<std::true_type, std::false_type,
                                     not_<can_invoke<quote<std::pair>, int, int, int>>>::value,
                "");
#endif
            static_assert(
                let<lazy::strict_or<std::true_type, std::true_type, std::false_type>>::value, "");
            static_assert(
                !let<lazy::strict_or<std::false_type, std::false_type, std::false_type>>::value,
                "");

            /**
             * \sa `meta::lazy::not_`
             */
            static_assert(let<lazy::not_<std::is_void<int>>>::value, "");

            /**
             * \sa `meta::lazy::if_`
             */
            static_assert(can_invoke<lambda<_a, lazy::if_<std::is_integral<_a>, _a>>, int>::value,
                          "");

            /**
             * \brief Test `meta::lazy::if_` to verify unselected branch is not evaluated, even if
             * ill-formed
             */
            template <typename T>
            using test_lazy_if_ = let<lazy::if_<std::is_void<T>, T, defer<std::pair, T>>>;
            static_assert(std::is_same<test_lazy_if_<void>, void>::value, "");
#if defined(META_WORKAROUND_GCC_64970)
            static_assert(
                !can_invoke<lambda<_a, lazy::if_<std::is_integral<_a>, _a>, float>>::value, "");
#endif

            /**
             * \sa `meta::lazy::if_c`
             */

            /**
             * \brief `meta::lazy::if_c` with `meta::defer`
             */
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

            /**
             * \brief `meta::lazy::if_c` with `meta::defer_i`
             */

            template <std::size_t N>
            struct fact2
              : let<lazy::if_c<
                    (N > 0), lazy::multiplies<meta::size_t<N>, defer_i<std::size_t, fact2, N - 1>>,
                    meta::size_t<1>>>
            {
            };

            static_assert(fact2<0>::value == 1, "");
            static_assert(fact2<1>::value == 1, "");
            static_assert(fact2<2>::value == 2, "");
            static_assert(fact2<3>::value == 6, "");
            static_assert(fact2<4>::value == 24, "");

            template <typename N>
            struct lazy_fact
              : let<lazy::if_c<(N::value > 0), lazy::multiplies<N, defer<fact, dec<N>>>,
                               meta::size_t<1>>>
            {
            };

            static_assert(lazy_fact<meta::size_t<0>>::value == 1, "");
            static_assert(lazy_fact<meta::size_t<1>>::value == 1, "");
            static_assert(lazy_fact<meta::size_t<2>>::value == 2, "");
            static_assert(lazy_fact<meta::size_t<3>>::value == 6, "");
            static_assert(lazy_fact<meta::size_t<4>>::value == 24, "");

            template <std::size_t N>
            struct lazy_fact2
              : let<lazy::if_c<
                    (N > 0),
                    lazy::multiplies<meta::size_t<N>, defer_i<std::size_t, lazy_fact2, N - 1>>,
                    meta::size_t<1>>>
            {
            };

            static_assert(lazy_fact2<0>::value == 1, "");
            static_assert(lazy_fact2<1>::value == 1, "");
            static_assert(lazy_fact2<2>::value == 2, "");
            static_assert(lazy_fact2<3>::value == 6, "");
            static_assert(lazy_fact2<4>::value == 24, "");
        } // namespace test_lazy_logical_group

    } // namespace test_logical_group

    namespace test_algorithm_group
    {
        namespace test_query_group
        {
            /**
             * \sa `meta::all_of`
             */
            static_assert(all_of<list<int, short, long>, quote_trait<std::is_integral>>::value, "");

            /**
             * \sa `meta::any_of`
             */
            static_assert(
                !any_of<list<int, short, long>, quote_trait<std::is_floating_point>>::value, "");
            static_assert(
                any_of<list<int, short, long, float>, quote_trait<std::is_floating_point>>::value,
                "");

            /**
             * \sa `meta::count`
             */
            using searchable_list = list<int, long, short, int>;

            static_assert(meta::count<searchable_list, int>::value == 2, "");
            static_assert(meta::count<searchable_list, short>::value == 1, "");
            static_assert(meta::count<searchable_list, double>::value == 0, "");

            /**
             * \sa `meta::count_if`
             */
            static_assert(
                meta::count_if<searchable_list, lambda<_a, std::is_same<_a, int>>>::value == 2, "");
            static_assert(
                meta::count_if<searchable_list, lambda<_b, std::is_same<_b, short>>>::value == 1,
                "");
            static_assert(
                meta::count_if<searchable_list, lambda<_c, std::is_same<_c, double>>>::value == 0,
                "");

            /**
             * \sa `meta::find`
             */
            static_assert(std::is_same<find<searchable_list, int>, searchable_list>::value, "");
            static_assert(
                std::is_same<find<searchable_list, short>, drop_c<searchable_list, 2>>::value, "");

            /**
             * \sa `meta::find_if`
             */
            static_assert(
                std::is_same<find_if<searchable_list, bind_front<quote<std::is_same>, int>>,
                             searchable_list>::value,
                "");
            static_assert(
                std::is_same<find_if<searchable_list, bind_front<quote<std::is_same>, double>>,
                             list<>>::value,
                "");

            /**
             * \sa `meta::find_index`
             */
            static_assert(find_index<searchable_list, int>::value == 0, "");
            static_assert(find_index<searchable_list, long>::value == 1, "");
            static_assert(find_index<searchable_list, short>::value == 2, "");
            static_assert(equal_to<find_index<searchable_list, double>, meta::npos>::value, "");
            static_assert(equal_to<find_index<searchable_list, float>, meta::npos>::value, "");

            static_assert(equal_to<find_index<list<>, double>, meta::npos>::value, "");

            using callable_find_index = lambda<_a, _b, lazy::find_index<_b, _a>>;
            using result = meta::invoke<callable_find_index, long, searchable_list>;
            static_assert(result{} == 1, "");

            /**
             * \sa `meta::in`
             */
            static_assert(in<list<int, int, short, float>, int>::value, "");
            static_assert(in<list<int, int, short, float>, short>::value, "");
            static_assert(in<list<int, int, short, float>, float>::value, "");
            static_assert(!in<list<int, int, short, float>, double>::value, "");

            /**
             * \sa `meta::none_of`
             */
            static_assert(
                none_of<list<int, short, long>, quote_trait<std::is_floating_point>>::value, "");

            /**
             * \sa `meta::reverse_find`
             */
            static_assert(std::is_same<reverse_find<searchable_list, int>, list<int>>::value, "");

            /**
             * \sa `meta::reverse_find_if`
             */
            static_assert(
                std::is_same<reverse_find_if<searchable_list, bind_front<quote<std::is_same>, int>>,
                             list<int>>::value,
                "");
            static_assert(std::is_same<reverse_find_if<searchable_list,
                                                       bind_front<quote<std::is_same>, double>>,
                                       list<>>::value,
                          "");

            /**
             * \sa `meta::reverse_find_index`
             */
            static_assert(reverse_find_index<searchable_list, int>::value == 3, "");
            static_assert(reverse_find_index<searchable_list, long>::value == 1, "");
            static_assert(reverse_find_index<searchable_list, short>::value == 2, "");
            static_assert(equal_to<reverse_find_index<searchable_list, double>, meta::npos>::value,
                          "");
            static_assert(equal_to<reverse_find_index<searchable_list, float>, meta::npos>::value,
                          "");

            static_assert(equal_to<reverse_find_index<list<>, double>, meta::npos>::value, "");

            namespace test_lazy_query_group
            {
                /**
                 * \sa `meta::lazy::all_of`
                 */
                static_assert(
                    _t<lazy::all_of<list<int, short, long>, quote_trait<std::is_integral>>>::value,
                    "");

                /**
                 * \sa `meta::lazy::any_of`
                 */
                static_assert(!_t<lazy::any_of<list<int, short, long>,
                                               quote_trait<std::is_floating_point>>>::value,
                              "");
                static_assert(_t<lazy::any_of<list<int, short, long, float>,
                                              quote<std::is_floating_point>>>::value,
                              "");

                /**
                 * \sa `meta::lazy::count`
                 */
                static_assert(let<lazy::count<searchable_list, int>>::value == 2, "");
                static_assert(let<lazy::count<searchable_list, short>>::value == 1, "");
                static_assert(let<lazy::count<searchable_list, double>>::value == 0, "");

                /**
                 * \sa `meta::lazy::count_if`
                 */
                static_assert(let<lazy::count_if<searchable_list,
                                                 lambda<_a, std::is_same<_a, int>>>>::value == 2,
                              "");
                static_assert(let<lazy::count_if<searchable_list,
                                                 lambda<_b, std::is_same<_b, short>>>>::value == 1,
                              "");
                static_assert(let<lazy::count_if<searchable_list,
                                                 lambda<_c, std::is_same<_c, double>>>>::value == 0,
                              "");

                /**
                 * \sa `meta::lazy::find`
                 */
                static_assert(
                    std::is_same<let<lazy::find<searchable_list, int>>, searchable_list>::value,
                    "");

                /**
                 * \sa `meta::lazy::find_if`
                 */
                static_assert(
                    std::is_same<
                        let<lazy::find_if<searchable_list, bind_front<quote<std::is_same>, int>>>,
                        searchable_list>::value,
                    "");
                static_assert(
                    std::is_same<find_if<searchable_list, bind_front<quote<std::is_same>, double>>,
                                 list<>>::value,
                    "");

                /**
                 * \sa `meta::lazy::find_index`
                 */
                static_assert(find_index<searchable_list, int>::value == 0, "");
                static_assert(find_index<searchable_list, long>::value == 1, "");
                static_assert(find_index<searchable_list, short>::value == 2, "");
                static_assert(equal_to<find_index<searchable_list, double>, meta::npos>::value, "");
                static_assert(equal_to<find_index<searchable_list, float>, meta::npos>::value, "");

                using empty_list = list<>;
                static_assert(equal_to<find_index<empty_list, double>, meta::npos>::value, "");

                using callable_find_index = lambda<_a, _b, lazy::find_index<_b, _a>>;
                using result = meta::invoke<callable_find_index, long, searchable_list>;
                static_assert(result{} == 1, "");

                /**
                 * \sa `meta::lazy::in`
                 */
                static_assert(let<lazy::in<list<int, int, short, float>, int>>::value, "");
                static_assert(let<lazy::in<list<int, int, short, float>, short>>::value, "");
                static_assert(let<lazy::in<list<int, int, short, float>, float>>::value, "");
                static_assert(!let<lazy::in<list<int, int, short, float>, double>>::value, "");

                /**
                 * \sa `meta::lazy::none_of`
                 */

                static_assert(let<lazy::none_of<list<int, short, long>,
                                                quote_trait<std::is_floating_point>>>::value,
                              "");

                /**
                 * \sa `meta::lazy::reverse_find`
                 */
                static_assert(
                    std::is_same<let<lazy::reverse_find<searchable_list, int>>, list<int>>::value,
                    "");

                /**
                 * \sa `meta::lazy::reverse_find_if`
                 */
                static_assert(
                    std::is_same<let<lazy::reverse_find_if<searchable_list,
                                                           bind_front<quote<std::is_same>, int>>>,
                                 list<int>>::value,
                    "");
                static_assert(
                    std::is_same<let<lazy::reverse_find_if<
                                     searchable_list, bind_front<quote<std::is_same>, double>>>,
                                 list<>>::value,
                    "");

                /**
                 * \sa `meta::lazy::reverse_find_index`
                 */
                static_assert(let<lazy::reverse_find_index<searchable_list, int>>::value == 3, "");
                static_assert(let<lazy::reverse_find_index<searchable_list, long>>::value == 1, "");
                static_assert(let<lazy::reverse_find_index<searchable_list, short>>::value == 2,
                              "");
                static_assert(equal_to<let<lazy::reverse_find_index<searchable_list, double>>,
                                       meta::npos>::value,
                              "");
                static_assert(equal_to<let<lazy::reverse_find_index<searchable_list, float>>,
                                       meta::npos>::value,
                              "");

                static_assert(
                    equal_to<let<lazy::reverse_find_index<list<>, double>>, meta::npos>::value, "");

                using callable_reverse_find_index =
                    lambda<_a, _b, lazy::reverse_find_index<_b, _a>>;
                using result = meta::invoke<callable_reverse_find_index, long, searchable_list>;
                static_assert(result{} == 1, "");
            } // namespace test_lazy_query_group

        } // namespace test_query_group

        namespace test_transformation_group
        {
            /**
             * \sa `meta::filter`
             */
            namespace detail
            {
                using mixed_unfiltered_list = list<int, double, short, float, long, char>;
                using unfiltered_int_list = list<int, short, long, char>;
                using unfilitered_fp_list = list<double, float>;
            } // namespace detail

            static_assert(std::is_same<detail::unfiltered_int_list,
                                       meta::filter<detail::mixed_unfiltered_list,
                                                    meta::quote<std::is_integral>>>::value,
                          "");
            static_assert(std::is_same<detail::unfilitered_fp_list,
                                       meta::filter<detail::mixed_unfiltered_list,
                                                    meta::quote<std::is_floating_point>>>::value,
                          "");

            /**
             * \sa `meta::accumulate`
             */
            static_assert(
                accumulate<as_list<meta::integer_range<int, 0, 11>>, int_<0>, quote<plus>>::value ==
                    55,
                "");

            /**
             * \sa `meta::cartesian_product`
             */
            static_assert(
                std::is_same<
                    cartesian_product<list<list<int_<0>, int_<1>>, list<int_<2>, int_<3>>>>,
                    list<list<int_<0>, int_<2>>, list<int_<0>, int_<3>>, list<int_<1>, int_<2>>,
                         list<int_<1>, int_<3>>>>::value,
                "");

            /**
             * \sa `meta::concat`
             */
            static_assert(std::is_same<concat<list<>, list<int_<5>, int_<10>>, list<void, int>>,
                                       list<int_<5>, int_<10>, void, int>>::value,
                          "");

            /**
             * \sa `meta::drop`
             */
            static_assert(std::is_same<drop<list<int, short, int, double, short, double, double>,
                                            meta::size_t<2>>,
                                       list<int, double, short, double, double>>::value,
                          "");

            /**
             * \sa `meta::drop_c`
             */
            static_assert(
                std::is_same<drop_c<list<int, short, int, double, short, double, double>, 2>,
                             list<int, double, short, double, double>>::value,
                "");

            /**
             * \sa `meta::join`
             */
            static_assert(std::is_same<join<list<list<>, list<int_<5>, int_<10>>, list<void, int>>>,
                                       list<int_<5>, int_<10>, void, int>>::value,
                          "");

            /**
             * \sa `meta::unique`
             */
            static_assert(

                std::is_same<meta::unique<list<int, short, int, double, short, double, double>>,
                             list<int, short, double>>::value,
                "");

            /**
             * \sa `meta::replace`
             */
            static_assert(
                std::is_same<
                    meta::replace<list<int, short, int, double, short, double, double>, int, float>,
                    list<float, short, float, double, short, double, double>>::value,
                "");

            /**
             * \sa `meta::replace_if`
             */
            static_assert(
                std::is_same<meta::replace_if<list<int, short, int, double, short, double, double>,
                                              quote<std::is_integral>, float>,
                             list<float, float, float, double, float, double, double>>::value,
                "");

            /**
             * \sa `meta::reverse`
             */
            static_assert(
                std::is_same<reverse<list<int, short, double>>, list<double, short, int>>::value,
                "");
            static_assert(std::is_same<reverse<list<int, short, double, float>>,
                                       list<float, double, short, int>>::value,
                          "");
            static_assert(
                std::is_same<reverse<list<int[1], int[2], int[3], int[4], int[5], int[6], int[7],
                                          int[8], int[9], int[10], int[11], int[12], int[13],
                                          int[14], int[15], int[16], int[17]>>,
                             list<int[17], int[16], int[15], int[14], int[13], int[12], int[11],
                                  int[10], int[9], int[8], int[7], int[6], int[5], int[4], int[3],
                                  int[2], int[1]>>::value,
                "");

            /**
             * \sa `meta::pop_front`
             */
            static_assert(
                std::is_same<pop_front<list<int, short, int, double, short, double, double>>,
                             list<short, int, double, short, double, double>>::value,
                "");

            /**
             * \sa `meta::push_back`
             */
            static_assert(
                std::is_same<push_back<list<int, short, int, double, short, double, double>, float>,
                             list<int, short, int, double, short, double, double, float>>::value,
                "");

            /**
             * \sa `meta::push_front
             */
            static_assert(
                std::is_same<
                    push_front<list<int, short, int, double, short, double, double>, float>,
                    list<float, int, short, int, double, short, double, double>>::value,
                "");

            /**
             * \sa `meta::sort`
             */

            using unsorted_list =
                list<char[5], char[3], char[2], char[6], char[1], char[5], char[10]>;
            using sorted_list =
                meta::sort<unsorted_list,
                           lambda<_a, _b, lazy::less<lazy::sizeof_<_a>, lazy::sizeof_<_b>>>>;
            using expected_list =
                list<char[1], char[2], char[3], char[5], char[5], char[6], char[10]>;
            static_assert(std::is_same<sorted_list, expected_list>::value, "");

            /**
             * \sa `meta::partition`
             */
            using unpartitioned_list =
                list<char[5], char[3], char[2], char[6], char[1], char[5], char[10]>;

            namespace detail
            {
                struct is_even
                {
                    template <typename N>
                    using invoke = bool_<N::type::value % 2 == 0>;
                };
                static_assert(!meta::invoke<not_fn<quote<is_even::invoke>>, int_<2>>::value, "");

                template <typename L>
                using get_size_t = let<var<_a, L>, lazy::size<_a>>;
            } // namespace detail

            using int_range_1_10_list = as_list<meta::integer_range<int, 1, 11>>;
            static_assert(meta::invoke<detail::is_even, int_<2>>::value, "");

            using even_numbers_first_list = partition<int_range_1_10_list, detail::is_even>;
            static_assert(
                std::is_same<even_numbers_first_list,
                             list<list<int_<2>, int_<4>, int_<6>, int_<8>, int_<10>>,
                                  list<int_<1>, int_<3>, int_<5>, int_<7>, int_<9>>>>::value,
                "");
            using char_arrays_list =
                list<char[5], char[3], char[2], char[6], char[1], char[5], char[10]>;
            using char_arrays_sz_gt_5_first_list =
                partition<char_arrays_list,
                          lambda<_a, lazy::greater<lazy::sizeof_<_a>, sizeof_<char[5]>>>>;

            static_assert(
                std::is_same<char_arrays_sz_gt_5_first_list,
                             list<list<char[6], char[10]>,
                                  list<char[5], char[3], char[2], char[1], char[5]>>>::value,
                "");

            /**
             * \sa `meta::fold`
             */

            static_assert(meta::fold<meta::as_list<meta::integer_range<std::size_t, 0, 5>>,
                                     meta::size_t<0>, meta::quote<meta::plus>>::value == 10,
                          "");

            static_assert(meta::fold<meta::as_list<meta::integer_range<std::size_t, 5, 10>>,
                                     meta::size_t<0>, meta::quote<meta::plus>>::value == 35,
                          "");

            static_assert(
                equal_to<meta::fold<meta::as_list<meta::integer_range<std::size_t, 0, 20>>,
                                    meta::size_t<0>, meta::quote<meta::plus>>,
                         meta::size_t<190>>::value,
                "");

            static_assert(std::is_same<meta::integer_range<std::size_t, 5, 10>,
                                       meta::integer_sequence<std::size_t, 5, 6, 7, 8, 9>>::value,
                          "");

            /**
             * \sa `meta::reverse_fold`
             */
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
            static_assert(
                std::is_same<rev<list<int, short, double>>, list<double, short, int>>::value, "");

            /**
             * \sa `meta::transform`
             */
            static_assert(std::is_same<transform<as_list<meta::integer_range<int, 0, 3>>,
                                                 lambda<_a, lazy::multiplies<_a, _a>>>,
                                       list<int_<0>, int_<1>, int_<4>>>::value,
                          "");

            static_assert(std::is_same<transform<as_list<meta::integer_range<int, 0, 3>>,
                                                 as_list<meta::integer_range<int, 4, 7>>,
                                                 lambda<_a, _b, lazy::multiplies<_a, _b>>>,
                                       list<int_<0>, int_<5>, int_<12>>>::value,
                          "");
            /**
             * \sa `meta::transpose`
             */
            static_assert(std::is_same<transpose<list<as_list<meta::integer_range<int, 0, 3>>,
                                                      as_list<meta::integer_range<int, 4, 7>>>>,
                                       list<list<int_<0>, int_<4>>, list<int_<1>, int_<5>>,
                                            list<int_<2>, int_<6>>>>::value,
                          "");

            /**
             * \sa `meta::zip`
             */
            static_assert(std::is_same<zip<list<as_list<meta::integer_range<char, 'a', 'd'>>,
                                                as_list<meta::integer_range<int, 4, 7>>>>,
                                       list<list<char_<'a'>, int_<4>>, list<char_<'b'>, int_<5>>,
                                            list<char_<'c'>, int_<6>>>>::value,
                          "");

            /**
             * \sa `meta::zip_with`
             */
            static_assert(std::is_same<zip_with<quote<plus>, list<list<int_<0>, int_<1>, int_<2>>,
                                                                  list<int_<4>, int_<5>, int_<6>>>>,
                                       list<int_<4>, int_<6>, int_<8>>>::value,
                          "");

            namespace test_lazy_transformation_group
            {
                /**
                 * \sa `meta::lazy::accumulate`
                 */
                static_assert(let<lazy::accumulate<as_list<meta::integer_range<int, 0, 11>>,
                                                   int_<0>, quote<plus>>>::value == 55,
                              "");

                /**
                 * \sa `meta::lazy::cartesian_product`
                 */
                static_assert(
                    std::is_same<let<lazy::cartesian_product<
                                     list<list<int_<0>, int_<1>>, list<int_<2>, int_<3>>>>>,
                                 list<list<int_<0>, int_<2>>, list<int_<0>, int_<3>>,
                                      list<int_<1>, int_<2>>, list<int_<1>, int_<3>>>>::value,
                    "");

                /**
                 * \sa `meta::lazy::concat`
                 */
                static_assert(
                    std::is_same<
                        let<lazy::concat<list<>, list<int_<5>, int_<10>>, list<void, int>>>,
                        list<int_<5>, int_<10>, void, int>>::value,
                    "");

                /**
                 * \sa `meta::lazy::drop`
                 */
                static_assert(
                    std::is_same<
                        let<lazy::drop<list<int, short, int, double, short, double, double>,
                                       meta::size_t<2>>>,
                        list<int, double, short, double, double>>::value,
                    "");

                /**
                 * \sa `meta::lazy::sort`
                 */
                using unsorted_list =
                    list<char[5], char[3], char[2], char[6], char[1], char[5], char[10]>;
                using lazy_sorted_list =
                    lazy::sort<unsorted_list,
                               lambda<_a, _b, lazy::less<lazy::sizeof_<_a>, lazy::sizeof_<_b>>>>;
                using expected_list =
                    list<char[1], char[2], char[3], char[5], char[5], char[6], char[10]>;
                static_assert(std::is_same<_t<lazy_sorted_list>, expected_list>::value, "");

                /**
                 * \sa `meta::lazy::partition`
                 */

                using lazy_partition_gt_len_5 = let<lazy::partition<
                    unpartitioned_list,
                    lambda<_a, lazy::greater<lazy::sizeof_<_a>, sizeof_<char[5]>>>>>;
                using lazy_partition_gt_len_1 = let<lazy::partition<
                    unpartitioned_list,
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

                /**
                 * \sa `meta::lazy::transform`
                 */
                template <class L>
                using cart_prod = reverse_fold<
                    L, list<list<>>,
                    lambda<_a, _b,
                           lazy::join<lazy::transform<
                               _b,
                               lambda<_c, lazy::join<lazy::transform<
                                              _a, lambda<_d, list<lazy::push_front<_d, _c>>>>>>>>>>;

                using CartProd = cart_prod<list<list<int, short>, list<float, double>>>;
                static_assert(
                    std::is_same<CartProd, list<list<int, float>, list<int, double>,
                                                list<short, float>, list<short, double>>>::value,
                    "");
                static_assert(
                    std::is_same<CartProd, cartesian_product<
                                               list<list<int, short>, list<float, double>>>>::value,
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
            using runtime_list = list<int, long, short>;
            constexpr auto r = meta::for_each(runtime_list{}, detail::check_integral());
            static_assert(
                std::is_same<_t<std::remove_cv<decltype(r)>>, detail::check_integral>::value, "");
        } // namespace test_runtime_group

    } // namespace test_algorithm_group

    namespace test_datatype_group
    {
        /**
         * \sa `meta::nil_`
         */
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

        /**
         * \sa `meta::inherit`
         */
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
            /**
             * \sa `meta::lazy::inherit`
             */
            using t3_lazy = lazy::inherit<list<detail::t1, detail::t2>>;
            static_assert(!std::is_base_of<detail::t1, t3_lazy>::value, "");
            static_assert(!std::is_base_of<detail::t2, t3_lazy>::value, "");

            static_assert(std::is_base_of<detail::t1, _t<t3_lazy>>::value, "");
            static_assert(std::is_base_of<detail::t2, _t<t3_lazy>>::value, "");
        } // namespace test_lazy_datatype_group

        namespace test_list_group
        {
            /**
             * \sa `meta::list`
             */
            static_assert(!std::is_same<list<int, char, void>, std::tuple<int, char, void>>::value,
                          "");

            /**
             * \sa `meta::as_list`
             */
            static_assert(
                std::is_same<list<int, char, void>, as_list<std::tuple<int, char, void>>>::value,
                "");
            static_assert(std::is_same<as_list<list<int, char, void>>,
                                       as_list<std::tuple<int, char, void>>>::value,
                          "");

            /**
             * \sa `meta::list::size()`
             */

            static_assert(list<int, char, void>::size() == size<list<int, char, void>>::value, "");

            /**
             * \sa `meta::at_c`
             */
            static_assert(std::is_same<at_c<list<int, char, void>, 2>, void>::value, "");

            /**
             * \sa `meta::at`
             */
            static_assert(std::is_same<at<list<int, char, void>, meta::size_t<0>>, int>::value, "");

#if defined(META_WORKAROUND_GCC_64970)
            static_assert(!can_invoke<quote<at>, list<int, char, void>, meta::size_t<3>>::value,
                          "");
#endif

            /**
             * \sa `meta::back`
             */
            static_assert(std::is_same<back<list<int, char, void>>, void>::value, "");

            /**
             * \sa `meta::front`
             */
            static_assert(std::is_same<front<list<int, char, void>>, int>::value, "");

            /**
             * \sa `meta::empty`
             */
            static_assert(empty<list<>>::value, "");

            /**
             * \sa `meta::size`
             */
            static_assert(size<as_list<meta::integer_range<int, 0, 10>>>::value == 10, "");

            /**
             * \sa `meta::npos`
             */
            static_assert(meta::size_t<std::numeric_limits<std::size_t>::max()>::value ==
                              meta::npos::value,
                          "");

            /**
             * \sa `meta::pair`
             */
            static_assert(can_invoke<quote<meta::pair>, int, double>::value, "");
#if defined(META_WORKAROUND_GCC_64970)
            static_assert(!can_invoke<quote<meta::pair>, int, double, int>::value, "");
#endif

            /**
             * \sa `meta::first`
             */
            static_assert(std::is_same<first<meta::pair<int, double>>, int>::value, "");
            /**
             * \sa `meta::second`
             */
            static_assert(std::is_same<second<meta::pair<int, double>>, double>::value, "");

            /**
             * \sa `meta::repeat_n`
             */
            static_assert(
                std::is_same<repeat_n<int_<5>, int>, list<int, int, int, int, int>>::value, "");

            /**
             * \sa `meta::repeat_n_c`
             */
            static_assert(std::is_same<repeat_n_c<5, int>, list<int, int, int, int, int>>::value,
                          "");

            namespace test_lazy_list_group
            {
                /**
                 * \sa `meta::lazy::as_list`
                 */
                static_assert(std::is_same<list<int, char, void>,
                                           let<lazy::as_list<std::tuple<int, char, void>>>>::value,
                              "");
                static_assert(std::is_same<let<lazy::as_list<list<int, char, void>>>,
                                           let<lazy::as_list<std::tuple<int, char, void>>>>::value,
                              "");

                /**
                 * \sa `meta::lazy::at`
                 */
                static_assert(
                    std::is_same<let<lazy::at<list<int, char, void>, meta::size_t<0>>>, int>::value,
                    "");

                /**
                 * \sa `meta::lazy::back`
                 */
                static_assert(std::is_same<let<lazy::back<list<int, char, void>>>, void>::value,
                              "");

                /**
                 * \sa `meta::lazy::front`
                 */
                static_assert(std::is_same<let<lazy::front<list<int, char, void>>>, int>::value,
                              "");

                /**
                 * \sa `meta::lazy::first`
                 */
                static_assert(std::is_same<let<lazy::first<list<int, char, void>>>, int>::value,
                              "");

                /**
                 * \sa `meta::lazy::second`
                 */
                static_assert(std::is_same<let<lazy::second<list<int, char, void>>>, char>::value,
                              "");

                /**
                 * \sa `meta::lazy::empty`
                 */
                static_assert(let<lazy::empty<list<>>>::value, "");

                /**
                 * \sa `meta::lazy::size`
                 */
                static_assert(let<lazy::size<as_list<meta::integer_range<int, 0, 10>>>>::value ==
                                  10,
                              "");

                /**
                 * \sa `meta::lazy::repeat_n`
                 */
                static_assert(std::is_same<let<lazy::repeat_n<int_<5>, char>>,
                                           list<char, char, char, char, char>>::value,
                              "");
            } // namespace test_lazy_list_group

        } // namespace test_list_group

        namespace test_integral_group
        {
#if META_CXX_VER >= META_CXX_STD_14
            /**
             * \sa `meta::make_integer_sequence`
             */
            static_assert(std::is_same<std::integer_sequence<int, 0, 1, 2>,
                                       meta::make_integer_sequence<int, 3>>::value,
                          "");
            /**
             * \sa `meta::make_index_sequence`
             */
            static_assert(std::is_same<std::integer_sequence<std::size_t, 0, 1, 2>,
                                       meta::make_index_sequence<3>>::value,
                          "");
            /**
             * \sa `meta::integer_sequence`
             */
            static_assert(std::is_same<std::integer_sequence<int, 1, 3, 2>,
                                       meta::integer_sequence<int, 1, 3, 2>>::value,
                          "");
            /**
             * \sa `meta::index_sequence`
             */
            static_assert(std::is_same<std::integer_sequence<std::size_t, 1, 3, 2>,
                                       meta::index_sequence<1, 3, 2>>::value,
                          "");
#endif

#if META_CXX_INTEGER_SEQUENCE
            static_assert(
                std::is_same<as_list<meta::make_index_sequence<3>>,
                             list<meta::size_t<0>, meta::size_t<1>, meta::size_t<2>>>::value,
                "");
            static_assert(std::is_same<as_list<meta::make_integer_sequence<int, 3>>,
                                       list<int_<0>, int_<1>, int_<2>>>::value,
                          "");
#endif

            /**
             * \sa `meta::bool_`
             */

            static_assert(equal_to<std::integral_constant<bool, true>, bool_<true>>::value, "");
            static_assert(equal_to<std::integral_constant<bool, false>, bool_<false>>::value, "");

            /**
             * \sa `meta::char_`
             */
            static_assert(equal_to<std::integral_constant<char, 'a'>, char_<'a'>>::value, "");
            /**
             * \sa `meta::int_`
             */
            static_assert(equal_to<std::integral_constant<int, 10>, int_<10>>::value, "");
            /**
             * \sa `meta::size_t`
             */
            static_assert(
                equal_to<std::integral_constant<std::size_t, 10>, meta::size_t<10>>::value, "");
            /**
             * \sa `meta::operator""_z`
             */
            static_assert(42_z == 42, "");
        } // namespace test_integral_group

        namespace test_extension_group
        {
        }

    } // namespace test_datatype_group

    namespace test_math_group
    {
        /**
         * \sa `meta::inc`
         */
        static_assert(equal_to<std::integral_constant<int, 2>, inc<int_<1>>>::value, "");
        /**
         * \sa `meta::dec`
         */
        static_assert(equal_to<std::integral_constant<int, 1>, dec<int_<2>>>::value, "");
        /**
         * \sa `meta::plus`
         */
        static_assert(equal_to<std::integral_constant<int, 3>, plus<int_<2>, int_<1>>>::value, "");
        static_assert(equal_to<plus<int_<1>, int_<2>>, plus<int_<2>, int_<1>>>::value, "");

        /**
         * \sa `meta::minus`
         */
        static_assert(equal_to<std::integral_constant<int, 1>, minus<int_<3>, int_<2>>>::value, "");
        static_assert(!equal_to<minus<int_<1>, int_<2>>, minus<int_<2>, int_<1>>>::value, "");

        /**
         * \sa `meta::multiplies`
         */
        static_assert(equal_to<std::integral_constant<int, 6>, multiplies<int_<3>, int_<2>>>::value,
                      "");
        static_assert(equal_to<multiplies<int_<3>, int_<2>>, multiplies<int_<2>, int_<3>>>::value,
                      "");

        /**
         * \sa `meta::divides`
         */
        static_assert(equal_to<std::integral_constant<int, 1>, divides<int_<3>, int_<2>>>::value,
                      "");
        static_assert(!equal_to<divides<int_<1>, int_<2>>, divides<int_<2>, int_<1>>>::value, "");

        /**
         * \sa `meta::negate`
         */
        static_assert(equal_to<int_<-1>, negate<int_<1>>>::value, "");

        /**
         * \sa `meta::modulus`
         */
        static_assert(equal_to<modulus<int_<10>, int_<2>>, int_<0>>::value, "");
        static_assert(equal_to<modulus<int_<11>, int_<2>>, int_<1>>::value, "");

        /**
         * \sa `meta::equal_to`
         */
        static_assert(equal_to<int_<100>, meta::size_t<100>>::value, "");
        static_assert(!std::is_same<int_<100>, meta::size_t<100>>::value, "");

        /**
         * \sa `meta::not_equal_to
         */
        static_assert(not_equal_to<int_<100>, int_<99>>::value, "");

        /**
         * \sa `meta::greater`
         */
        static_assert(greater<int_<11>, int_<10>>::value, "");
        static_assert(!greater<int_<11>, int_<11>>::value, "");
        static_assert(!greater<int_<11>, int_<12>>::value, "");

        /**
         * \sa `meta::greater_equal`
         */
        static_assert(greater_equal<int_<11>, int_<10>>::value, "");
        static_assert(greater_equal<int_<11>, int_<11>>::value, "");
        static_assert(!greater_equal<int_<11>, int_<12>>::value, "");

        /**
         * \sa `meta::less`
         */
        static_assert(!less<int_<11>, int_<10>>::value, "");
        static_assert(!less<int_<11>, int_<11>>::value, "");
        static_assert(less<int_<11>, int_<12>>::value, "");

        /**
         * \sa `meta::less_equal`
         */
        static_assert(!less_equal<int_<11>, int_<10>>::value, "");
        static_assert(less_equal<int_<11>, int_<11>>::value, "");
        static_assert(less_equal<int_<11>, int_<12>>::value, "");

        /**
         * \sa `meta::bit_and`
         */
        static_assert(std::is_same<int_<(10 & 15)>, bit_and<int_<10>, int_<15>>>::value, "");
        static_assert(std::is_same<int_<(1 & 2)>, bit_and<int_<1>, int_<2>>>::value, "");

        /**
         * \sa `meta::bit_or`
         */
        static_assert(std::is_same<int_<(10 | 15)>, bit_or<int_<10>, int_<15>>>::value, "");
        static_assert(std::is_same<int_<(2 | 1)>, bit_or<int_<2>, int_<1>>>::value, "");

        /**
         * \sa `meta::bit_xor`
         */
        static_assert(std::is_same<int_<(1 ^ 1)>, bit_xor<int_<1>, int_<1>>>::value, "");
        static_assert(std::is_same<int_<(10 ^ 15)>, bit_xor<int_<10>, int_<15>>>::value, "");

        /**
         * \sa `meta::bit_not`
         */
        static_assert(equal_to<int_<(~(15))>, bit_not<int_<15>>>::value, "");
        static_assert(equal_to<int_<(~(0))>, bit_not<int_<0>>>::value, "");

        /**
         * \sa `meta::min`
         */
        static_assert(meta::min<meta::size_t<0>, meta::size_t<1>>::value == 0, "");
        static_assert(meta::min<meta::size_t<0>, meta::size_t<0>>::value == 0, "");
        static_assert(meta::min<meta::size_t<1>, meta::size_t<0>>::value == 0, "");

        /**
         * \sa `meta::max`
         */
        static_assert(meta::max<meta::size_t<0>, meta::size_t<1>>::value == 1, "");
        static_assert(meta::max<meta::size_t<1>, meta::size_t<0>>::value == 1, "");
        static_assert(meta::max<meta::size_t<1>, meta::size_t<1>>::value == 1, "");

        namespace test_lazy_math_group
        {
            /**
             * \sa `meta::lazy::inc`
             */
            static_assert(std::is_same<int_<2>, let<lazy::inc<int_<1>>>>::value, "");

            /**
             * \sa `meta::lazy::dec`
             */
            static_assert(std::is_same<int_<0>, let<lazy::dec<int_<1>>>>::value, "");

            /**
             * \sa `meta::lazy::plus`
             */
            static_assert(std::is_same<int_<5>, let<lazy::plus<int_<3>, int_<2>>>>::value, "");

            /**
             * \sa `meta::lazy::minus`
             */
            static_assert(std::is_same<int_<-2>, let<lazy::minus<int_<0>, int_<2>>>>::value, "");

            /**
             * \sa `meta::lazy::equal_to`
             */
            static_assert(std::is_same<int_<-10>, let<lazy::multiplies<int_<-5>, int_<2>>>>::value,
                          "");

            /**
             * \sa `meta::lazy::multiplies`
             */
            static_assert(std::is_same<let<lazy::multiplies<int_<2>, int_<3>>>,
                                       let<lazy::multiplies<int_<3>, int_<2>>>>::value,
                          "");

            /**
             * \sa `meta::lazy::divides`
             */
            static_assert(std::is_same<int_<1>, let<lazy::divides<int_<3>, int_<2>>>>::value, "");
            static_assert(!std::is_same<divides<int_<2>, int_<3>>,
                                        let<lazy::divides<int_<3>, int_<2>>>>::value,
                          "");

            /**
             * \sa `meta::lazy::negate`
             */
            static_assert(equal_to<int_<1>, let<lazy::negate<int_<-1>>>>::value, "");

            /**
             * \sa `meta::lazy::modulus`
             */
            static_assert(std::is_same<int_<0>, let<lazy::modulus<int_<10>, int_<2>>>>::value, "");
            static_assert(std::is_same<int_<1>, let<lazy::modulus<int_<11>, int_<2>>>>::value, "");

            /**
             * \sa `meta::lazy::equal_to`
             */
            static_assert(let<lazy::equal_to<int_<100>, meta::size_t<100>>>::value, "");

            /**
             * \sa `meta::lazy::not_equal_to
             */
            static_assert(let<lazy::not_equal_to<int_<100>, int_<99>>>::value, "");

            /**
             * \sa `meta::lazy::greater`
             */
            static_assert(let<lazy::greater<int_<11>, int_<10>>>::value, "");
            static_assert(!let<lazy::greater<int_<11>, int_<12>>>::value, "");

            /**
             * \sa `meta::lazy::greater_equal`
             */
            static_assert(let<lazy::greater_equal<int_<11>, int_<10>>>::value, "");
            static_assert(let<lazy::greater_equal<int_<11>, int_<11>>>::value, "");
            static_assert(!let<lazy::greater_equal<int_<11>, int_<12>>>::value, "");

            /**
             * \sa `meta::lazy::less`
             */
            static_assert(!let<lazy::less<int_<11>, int_<10>>>::value, "");
            static_assert(!let<lazy::less<int_<11>, int_<11>>>::value, "");
            static_assert(let<lazy::less<int_<11>, int_<12>>>::value, "");

            /**
             * \sa `meta::lazy::less_equal`
             */
            static_assert(!let<lazy::less_equal<int_<11>, int_<10>>>::value, "");
            static_assert(let<lazy::less_equal<int_<11>, int_<11>>>::value, "");
            static_assert(let<lazy::less_equal<int_<11>, int_<12>>>::value, "");

            /**
             * \sa `meta::lazy::bit_and`
             */
            static_assert(
                std::is_same<int_<(10 & 15)>, let<lazy::bit_and<int_<10>, int_<15>>>>::value, "");
            static_assert(std::is_same<int_<(1 & 2)>, let<lazy::bit_and<int_<1>, int_<2>>>>::value,
                          "");

            /**
             * \sa `meta::lazy::bit_or`
             */
            static_assert(
                std::is_same<int_<(10 | 15)>, let<lazy::bit_or<int_<10>, int_<15>>>>::value, "");
            static_assert(std::is_same<int_<(2 | 1)>, let<lazy::bit_or<int_<2>, int_<1>>>>::value,
                          "");

            /**
             * \sa `meta::lazy::bit_xor`
             */
            static_assert(std::is_same<int_<(1 ^ 1)>, let<lazy::bit_xor<int_<1>, int_<1>>>>::value,
                          "");
            static_assert(
                std::is_same<int_<(10 ^ 15)>, let<lazy::bit_xor<int_<10>, int_<15>>>>::value, "");

            /**
             * \sa `meta::lazy::bit_not`
             */
            static_assert(equal_to<int_<(~(15))>, let<lazy::bit_not<int_<15>>>>::value, "");
            static_assert(equal_to<int_<(~(0))>, let<lazy::bit_not<int_<0>>>>::value, "");

        } // namespace test_lazy_math_group

    } // namespace test_math_group
} // namespace test_meta_group

int main()
{
    test_tuple_cat();
    return ::test_result();
}
