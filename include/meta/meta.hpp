/// \file meta.hpp Tiny meta-programming library.
//
// Meta library
//
//  Copyright Eric Niebler 2014-2015
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/meta
//

#ifndef META_HPP
#define META_HPP

#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <meta/meta_fwd.hpp>

/// \defgroup meta Meta
///
/// A tiny metaprogramming library

/// \defgroup metafunction Metafunction
/// Metafunction invocation/composition.
/// \ingroup meta

/// \defgroup invocation Invocation
/// Metafunction invocation
/// \ingroup metafunction

/// \defgroup composition Composition
/// Metafunction composition
/// \ingroup metafunction

/// \defgroup logical Logical
/// Logical operations
/// \ingroup meta

/// \defgroup algorithm Algorithms
/// Algorithms.
/// \ingroup meta

/// \defgroup query Query/Search
/// Query and search algorithms
/// \ingroup algorithm

/// \defgroup transformation Transformation
/// Transformation algorithms
/// \ingroup algorithm

/// \defgroup runtime Runtime
/// Runtime algorithms
/// \ingroup algorithm

/// \defgroup datatype Datatype
/// Datatypes.
/// \ingroup meta

/// \defgroup list List
/// \ingroup datatype

/// \defgroup integral Integer sequence
/// Equivalent to C++14's `std::integer_sequence`
/// \ingroup datatype

/// \defgroup extension Extension
/// Extend meta with your own datatypes.
/// \ingroup datatype

/// \defgroup math Math
/// Integral constant arithmetic.
/// \ingroup meta

/// \defgroup lazy_metafunction lazy
/// \ingroup metafunction

/// \defgroup lazy_invocation lazy
/// \ingroup invocation

/// \defgroup lazy_composition lazy
/// \ingroup composition

/// \defgroup lazy_logical lazy
/// \ingroup logical

/// \defgroup lazy_query lazy
/// \ingroup query

/// \defgroup lazy_transformation lazy
/// \ingroup transformation

/// \defgroup lazy_list lazy
/// \ingroup list

/// \defgroup lazy_math lazy
/// \ingroup math

/// Tiny metaprogramming library
namespace meta
{
    /// \cond
    inline namespace v1
    {
        /// \endcond

        template <template <typename...> class C, typename... Ts>
        struct defer;

        namespace detail
        {
            /// Returns a \p T nullptr
            template <typename T>
            constexpr T *_nullptr_v()
            {
                return nullptr;
            }
        } // namespace detail

        /// An empty type.
        /// \ingroup datatype
        struct nil_
        {
        };

        /// "Evaluate" the metafunction \p T by returning the nested \c T::type
        /// alias.
        /// \ingroup invocation
        template <typename T>
        using eval = typename T::type;

        /// Evaluate the Metafunction Class \p F with the arguments \p Args.
        /// \ingroup invocation
        template <typename F, typename... Args>
        using apply = typename F::template apply<Args...>;

        /// Lazy versions of meta actions
        namespace lazy
        {
            /// \sa `meta::eval`
            /// \ingroup lazy_invocation
            template <typename T>
            using eval = defer<eval, T>;

            /// \sa `meta::apply`
            /// \ingroup lazy_invocation
            template <typename F, typename... Args>
            using apply = defer<apply, F, Args...>;
        }

        /// A Metafunction Class that always returns \p T.
        /// \ingroup metafunction
        template <typename T>
        struct always
        {
        private:
            // Redirect through a class template for compilers that have not
            // yet implemented CWG 1558:
            // <http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#1558>
            template <typename...>
            struct impl
            {
                using type = T;
            };

        public:
            template <typename... Ts>
            using apply = eval<impl<Ts...>>;
        };

        /// An alias for `void`.
        /// \ingroup metafunction
        template <typename... Ts>
        using void_ = apply<always<void>, Ts...>;

        namespace lazy
        {
            /// \sa `meta::always`
            /// \ingroup lazy_metafunction
            template <typename T>
            using always = defer<always, T>;
        }

        /// \cond
        namespace detail
        {
            template <typename, typename = void>
            struct has_type_
            {
                using type = std::false_type;
            };

            template <typename T>
            struct has_type_<T, void_<typename T::type>>
            {
                using type = std::true_type;
            };

            template <template <typename...> class C, typename, typename = void>
            struct defer_
            {
            };

            template <template <typename...> class C, typename... Ts>
            struct defer_<C, list<Ts...>, void_<C<Ts...>>>
            {
                using type = C<Ts...>;
            };
        } // namespace detail
        /// \endcond

        /// An alias for `std::true_type` if `T::type` exists and names a
        /// type; otherwise, it's an alias for `std::false_type`.
        /// \ingroup metafunction
        template <typename T>
        using has_type = eval<detail::has_type_<T>>;

        ///////////////////////////////////////////////////////////////////////////////////////////
        // defer
        /// A wrapper that defers the instantiation of a template in a \c lambda
        /// expression.
        ///
        /// In the code below, the lambda would ideally be written as
        /// `lambda<_a,_b,push_back<_a,_b>>`, however this fails since `push_back`
        /// expects its first argument to be a list, not a placeholder. Instead,
        /// we express it using \c defer as follows:
        ///
        /// \code
        /// template<typename List>
        /// using reverse = reverse_fold<List, list<>, lambda<_a, _b, defer<push_back, _a, _b>>>;
        /// \endcode
        ///
        /// \ingroup invocation
        template <template <typename...> class C, typename... Ts>
        struct defer : detail::defer_<C, list<Ts...>>
        {
        };

        /// An integral constant wrapper for \c std::size_t.
        /// \ingroup integral
        template <std::size_t N>
        using size_t = std::integral_constant<std::size_t, N>;

        /// A metafunction that computes the size of the type \p T.
        /// \par Complexity
        /// \f$ O(1) \f$.
        /// \ingroup metafunction
        template <class T>
        using sizeof_ = meta::size_t<sizeof(T)>;

        /// A metafunction that computes the alignment required for
        /// any instance of the type \p T.
        /// \par Complexity
        /// \f$ O(1) \f$.
        /// \ingroup metafunction
        template <class T>
        using alignof_ = meta::size_t<alignof(T)>;

        namespace lazy
        {
            /// \sa `meta::sizeof_`
            /// \ingroup lazy_metafunction
            template <typename T>
            using sizeof_ = defer<sizeof_, T>;

            /// \sa `meta::alignof_`
            /// \ingroup lazy_metafunction
            template <typename T>
            using alignof_ = defer<alignof_, T>;
        }

        /// An integral constant wrapper for \c bool.
        /// \ingroup integral
        template <bool B>
        using bool_ = std::integral_constant<bool, B>;

        /// A metafunction that always returns its argument \p T.
        /// \ingroup metafunction
        template <typename T>
        struct id
        {
            using type = T;
        };

        /// A metafunction that is type \p T.
        /// \ingroup metafunction
        template <typename T>
        using id_t = eval<id<T>>;

        namespace lazy
        {
            /// \sa `meta::id`
            /// \ingroup lazy_metafunction
            template <typename T>
            using id = defer<id, T>;
        }

        /// Turn a class template or alias template \p C into a
        /// Metafunction Class.
        /// \ingroup composition
        template <template <typename...> class C>
        struct quote
        {
        private:
            template <typename, typename = quote, typename = void>
            struct impl
            {
            };
            template <typename... Ts, template <typename...> class D>
            struct impl<list<Ts...>, quote<D>, void_<D<Ts...>>>
            {
                using type = D<Ts...>;
            };

        public:
            // Indirection here needed to avoid Core issue 1430
            // http://open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#1430
            template <typename... Ts>
            using apply = eval<impl<list<Ts...>>>;
        };

        /// Turn a class template or alias template \p F taking literals of
        /// type \p T into a Metafunction Class.
        /// \ingroup composition
        template <typename T, template <T...> class F>
        struct quote_i
        {
        private:
            template <typename, typename = quote_i, typename = void>
            struct impl
            {
            };
            template <typename... Ts, typename U, template <U...> class D>
            struct impl<list<Ts...>, quote_i<U, D>, void_<D<Ts::type::value...>>>
            {
                using type = D<Ts::type::value...>;
            };

        public:
            // Indirection here needed to avoid Core issue 1430
            // http://open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#1430
            template <typename... Ts>
            using apply = eval<impl<list<Ts...>>>;
        };

        /// Turn a metafunction \p C into a Metafunction Class.
        /// \ingroup composition
        template <template <typename...> class C>
        struct quote_trait
        {
            template <typename... Ts>
            using apply = eval<apply<quote<C>, Ts...>>;
        };

        /// Turn a metafunction \p C taking literals of type \p T into a
        /// Metafunction Class.
        /// \ingroup composition
        template <typename T, template <T...> class C>
        struct quote_trait_i
        {
            template <typename... Ts>
            using apply = eval<apply<quote_i<T, C>, Ts...>>;
        };

        /// Compose the Metafunction Classes \p Fs in the parameter pack
        /// \p Ts.
        /// \ingroup composition
        template <typename... Fs>
        struct compose
        {
        };

        template <typename F0>
        struct compose<F0>
        {
            template <typename... Ts>
            using apply = apply<F0, Ts...>;
        };

        template <typename F0, typename... Fs>
        struct compose<F0, Fs...>
        {
            template <typename... Ts>
            using apply = apply<F0, apply<compose<Fs...>, Ts...>>;
        };

        namespace lazy
        {
            /// \sa 'meta::compose'
            /// \ingroup lazy_composition
            template <typename... Fns>
            using compose = defer<compose, Fns...>;
        }

        /// A Metafunction Class that partially applies the Metafunction Class
        /// \p F by binding the arguments \p Ts to the \e front of \p F.
        /// \ingroup composition
        template <typename F, typename... Ts>
        struct bind_front
        {
            template <typename... Us>
            using apply = apply<F, Ts..., Us...>;
        };

        /// A Metafunction Class that partially applies the Metafunction Class
        /// \p F by binding the arguments \p Us to the \e back of \p F.
        /// \ingroup composition
        template <typename F, typename... Us>
        struct bind_back
        {
            template <typename... Ts>
            using apply = apply<F, Ts..., Us...>;
        };

        namespace lazy
        {
            /// \sa 'meta::bind_front'
            /// \ingroup lazy_composition
            template <typename Fn, typename... Ts>
            using bind_front = defer<bind_front, Fn, Ts...>;

            /// \sa 'meta::bind_back'
            /// \ingroup lazy_composition
            template <typename Fn, typename... Ts>
            using bind_back = defer<bind_back, Fn, Ts...>;
        }

        /// Extend meta with your own datatypes.
        namespace extension
        {
            /// A metafunction that unpacks the types in the type list
            /// \p List into the Metafunction Class \p F.
            /// \ingroup extension
            template <typename F, typename List>
            struct apply_list
            {
            };

            template <typename F, template <typename...> class T, typename... Ts>
            struct apply_list<F, T<Ts...>> : lazy::apply<F, Ts...>
            {
            };

            template <typename F, typename T, T... Is>
            struct apply_list<F, integer_sequence<T, Is...>>
                : lazy::apply<F, std::integral_constant<T, Is>...>
            {
            };
        }

        /// Applies the Metafunction Class \p C using the types in
        /// the type list \p List as arguments.
        /// \ingroup invocation
        template <typename C, typename List>
        using apply_list = eval<extension::apply_list<C, List>>;

        namespace lazy
        {
            template <typename F, typename List>
            using apply_list = defer<apply_list, F, List>;
        }

        /// A Metafunction Class that takes a bunch of arguments, bundles them
        /// into a type list, and then calls the Metafunction Class \p F with the
        /// type list \p Q.
        /// \ingroup composition
        template <typename F, typename Q = quote<list>>
        using curry = compose<F, Q>;

        /// A Metafunction Class that takes a type list, unpacks the types, and
        /// then calls the Metafunction Class \p F with the types.
        /// \ingroup composition
        template <typename F>
        using uncurry = bind_front<quote<apply_list>, F>;

        namespace lazy
        {
            /// \sa 'meta::curry'
            /// \ingroup lazy_composition
            template <typename F, typename Q = quote<list>>
            using curry = defer<curry, F, Q>;

            /// \sa 'meta::uncurry'
            /// \ingroup lazy_composition
            template <typename F>
            using uncurry = defer<uncurry, F>;
        }

        /// A Metafunction Class that reverses the order of the first two arguments.
        /// \ingroup composition
        template <typename F>
        struct flip
        {
        private:
            template <typename... Ts>
            struct impl
            {
            };
            template <typename A, typename B, typename... Ts>
            struct impl<A, B, Ts...> : lazy::apply<F, B, A, Ts...>
            {
            };

        public:
            template <typename... Ts>
            using apply = eval<impl<Ts...>>;
        };

        namespace lazy
        {
            /// \sa 'meta::flip'
            /// \ingroup lazy_composition
            template <typename F>
            using flip = defer<flip, F>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // if_
        /// \cond
        namespace detail
        {
            template <typename...>
            struct _if_
            {
            };

            template <typename If, typename Then>
            struct _if_<If, Then> : std::enable_if<If::type::value, Then>
            {
            };

            template <typename If, typename Then, typename Else>
            struct _if_<If, Then, Else> : std::conditional<If::type::value, Then, Else>
            {
            };
        } // namespace detail
          /// \endcond

        /// Select one type or another depending on a compile-time Boolean.
        /// \ingroup logical
        template <typename... Args>
        using if_ = eval<detail::_if_<Args...>>;

        /// Select one type or another depending on a compile-time Boolean.
        /// \ingroup logical
        template <bool If, typename... Args>
        using if_c = eval<detail::_if_<bool_<If>, Args...>>;

        namespace lazy
        {
            /// \sa 'meta::if_'
            /// \ingroup lazy_logical
            template <typename... Args>
            using if_ = defer<if_, Args...>;
        }

        /// \cond
        namespace detail
        {
            template <typename... Bools>
            struct _and_;

            template <>
            struct _and_<> : std::true_type
            {
            };

            template <typename Bool, typename... Bools>
            struct _and_<Bool, Bools...>
                : if_c<!Bool::type::value, std::false_type, _and_<Bools...>>
            {
            };

            template <typename... Bools>
            struct _or_;

            template <>
            struct _or_<> : std::false_type
            {
            };

            template <typename Bool, typename... Bools>
            struct _or_<Bool, Bools...> : if_c<Bool::type::value, std::true_type, _or_<Bools...>>
            {
            };
        } // namespace detail
        /// \endcond

        /// Logically negate the Boolean parameter
        /// \ingroup logical
        template <bool Bool>
        using not_c = bool_<!Bool>;

        /// Logically negate the integral constant-wrapped Boolean parameter.
        /// \ingroup logical
        template <typename Bool>
        using not_ = not_c<Bool::type::value>;

        /// Logically and together all the Boolean parameters
        /// \ingroup logical
        template <bool... Bools>
        using and_c = std::is_same<integer_sequence<bool, Bools...>,
                                   integer_sequence<bool, (Bools || true)...>>;

        /// Logically and together all the integral constant-wrapped Boolean
        /// parameters, \e without doing short-circuiting.
        /// \ingroup logical
        template <typename... Bools>
        using fast_and = and_c<Bools::type::value...>;

        /// Logically and together all the integral constant-wrapped Boolean
        /// parameters, \e with short-circuiting.
        /// \ingroup logical
        template <typename... Bools>
        using and_ = eval<detail::_and_<Bools...>>;

        /// Logically or together all the Boolean parameters
        /// \ingroup logical
        template <bool... Bools>
        using or_c = not_<std::is_same<integer_sequence<bool, Bools...>,
                                       integer_sequence<bool, (Bools && false)...>>>;

        /// Logically or together all the integral constant-wrapped Boolean
        /// parameters, \e without doing short-circuiting.
        /// \ingroup logical
        template <typename... Bools>
        using fast_or = or_c<Bools::type::value...>;

        /// Logically or together all the integral constant-wrapped Boolean
        /// parameters, \e with short-circuiting.
        /// \ingroup logical
        template <typename... Bools>
        using or_ = eval<detail::_or_<Bools...>>;

        namespace lazy
        {
            /// \sa 'meta::and_'
            /// \ingroup lazy_logical
            template <typename... Bools>
            using and_ = defer<and_, Bools...>;

            /// \sa 'meta::or_'
            /// \ingroup lazy_logical
            template <typename... Bools>
            using or_ = defer<or_, Bools...>;

            /// \sa 'meta::not_'
            /// \ingroup lazy_logical
            template <typename Bool>
            using not_ = defer<not_, Bool>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // list
        /// A list of types.
        /// \ingroup list
        template <typename... Ts>
        struct list
        {
            using type = list;
            /// \return `sizeof...(Ts)`
            static constexpr std::size_t size() noexcept { return sizeof...(Ts); }
        };

        ///////////////////////////////////////////////////////////////////////////////////////////
        // size
        /// An integral constant wrapper that is the size of the \c meta::list
        /// \p List.
        /// \ingroup list
        template <typename List>
        using size = meta::size_t<List::size()>;

        namespace lazy
        {
            /// \sa 'meta::size'
            /// \ingroup lazy_list
            template <typename List>
            using size = defer<size, List>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // concat
        /// \cond
        namespace detail
        {
            template <typename... Lists>
            struct concat_
            {
            };

            template <>
            struct concat_<>
            {
                using type = list<>;
            };

            template <typename... List1>
            struct concat_<list<List1...>>
            {
                using type = list<List1...>;
            };

            template <typename... List1, typename... List2>
            struct concat_<list<List1...>, list<List2...>>
            {
                using type = list<List1..., List2...>;
            };

            template <typename... List1, typename... List2, typename... List3>
            struct concat_<list<List1...>, list<List2...>, list<List3...>>
            {
                using type = list<List1..., List2..., List3...>;
            };

            template <typename... List1, typename... List2, typename... List3, typename... Rest>
            struct concat_<list<List1...>, list<List2...>, list<List3...>, Rest...>
                : concat_<list<List1..., List2..., List3...>, Rest...>
            {
            };
        } // namespace detail
        /// \endcond

        /// Concatenates several lists into a single list.
        /// \pre The parameters must all be instantiations of \c meta::list.
        /// \par Complexity
        /// \f$ O(L) \f$ where \f$ L \f$ is the number of lists in the list of lists.
        /// \ingroup transformation
        template <typename... Lists>
        using concat = eval<detail::concat_<Lists...>>;

        namespace lazy
        {
            /// \sa 'meta::concat'
            /// \ingroup lazy_transformation
            template <typename... Lists>
            using concat = defer<concat, Lists...>;
        }

        /// Joins a list of lists into a single list.
        /// \pre The parameter must be an instantiation of \c meta::list\<T...\>
        /// where each \c T is itself an instantiation of \c meta::list.
        /// \par Complexity
        /// \f$ O(L) \f$ where \f$ L \f$ is the number of lists in the list of
        /// lists.
        /// \ingroup transformation
        template <typename ListOfLists>
        using join = apply_list<quote<concat>, ListOfLists>;

        namespace lazy
        {
            /// \sa 'meta::join'
            /// \ingroup lazy_transformation
            template <typename ListOfLists>
            using join = defer<join, ListOfLists>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // repeat_n
        /// \cond
        namespace detail
        {
            template <std::size_t N, typename T>
            struct repeat_n_c_
            {
                using type = concat<eval<repeat_n_c_<N / 2, T>>, eval<repeat_n_c_<N / 2, T>>,
                                    eval<repeat_n_c_<N % 2, T>>>;
            };

            template <typename T>
            struct repeat_n_c_<0, T>
            {
                using type = list<>;
            };

            template <typename T>
            struct repeat_n_c_<1, T>
            {
                using type = list<T>;
            };
        } // namespace detail
        /// \endcond

        /// Generate `list<T,T,T...T>` of size \p N arguments.
        /// \par Complexity
        /// \f$ O(log N) \f$.
        /// \ingroup list
        template <typename N, typename T = void>
        using repeat_n = eval<detail::repeat_n_c_<N::type::value, T>>;

        /// Generate `list<T,T,T...T>` of size \p N arguments.
        /// \par Complexity
        /// \f$ O(log N) \f$.
        /// \ingroup list
        template <std::size_t N, typename T = void>
        using repeat_n_c = eval<detail::repeat_n_c_<N, T>>;

        namespace lazy
        {
            /// \sa 'meta::repeat_n'
            /// \ingroup lazy_list
            template <typename N, typename T = void>
            using repeat_n = defer<repeat_n, N, T>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // list_element
        /// \cond
        namespace detail
        {
            template <typename VoidPtrs>
            struct list_element_impl_;

            template <typename... VoidPtrs>
            struct list_element_impl_<list<VoidPtrs...>>
            {
                static nil_ eval(...);

                template <typename T, typename... Us>
                static T eval(VoidPtrs..., T *, Us *...);
            };

            template <typename N, typename List>
            struct list_element_
            {
            };

            template <typename N, typename... Ts>
            struct list_element_<N, list<Ts...>>
                : decltype(list_element_impl_<repeat_n<N, void *>>::eval(
                      detail::_nullptr_v<id<Ts>>()...))
            {
            };
        } // namespace detail
        /// \endcond

        ///////////////////////////////////////////////////////////////////////////////////////////
        // list_element
        /// Return the \p N th element in the \c meta::list \p List.
        /// \par Complexity
        /// Amortized \f$ O(1) \f$.
        /// \ingroup list
        template <typename N, typename List>
        using list_element = eval<detail::list_element_<N, List>>;

        /// Return the \p N th element in the \c meta::list \p List.
        /// \par Complexity
        /// Amortized \f$ O(1) \f$.
        /// \ingroup list
        template <std::size_t N, typename List>
        using list_element_c = list_element<meta::size_t<N>, List>;

        namespace lazy
        {
            /// \sa 'meta::list_element'
            /// \ingroup lazy_list
            template <typename N, typename List>
            using list_element = defer<list_element, N, List>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // drop
        /// \cond
        namespace detail
        {
            ///////////////////////////////////////////////////////////////////////////////////////
            /// drop_impl_
            template <typename VoidPtrs>
            struct drop_impl_
            {
                static nil_ eval(...);
            };

            template <typename... VoidPtrs>
            struct drop_impl_<list<VoidPtrs...>>
            {
                static nil_ eval(...);

                template <typename... Ts>
                static id<list<Ts...>> eval(VoidPtrs..., id<Ts> *...);
            };

            template <typename N, typename List>
            struct drop_
            {
            };

            template <typename N, typename... Ts>
            struct drop_<N, list<Ts...>>
                : decltype(drop_impl_<repeat_n<N, void *>>::eval(detail::_nullptr_v<id<Ts>>()...))
            {
            };
        } // namespace detail
        /// \endcond

        /// Return a new \c meta::list by removing the first \p N elements from \p List.
        /// \par Complexity
        /// \f$ O(1) \f$.
        /// \ingroup transformation
        template <typename N, typename List>
        using drop = eval<detail::drop_<N, List>>;

        /// Return a new \c meta::list by removing the first \p N elements from \p List.
        /// \par Complexity
        /// \f$ O(1) \f$.
        /// \ingroup transformation
        template <std::size_t N, typename List>
        using drop_c = eval<detail::drop_<meta::size_t<N>, List>>;

        namespace lazy
        {
            /// \sa 'meta::drop'
            /// \ingroup lazy_transformation
            template <typename N, typename List>
            using drop = defer<drop, N, List>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // front
        /// \cond
        namespace detail
        {
            template <typename List>
            struct front_
            {
            };

            template <typename Head, typename... List>
            struct front_<list<Head, List...>>
            {
                using type = Head;
            };
        } // namespace detail
        /// \endcond

        /// Return the first element in \c meta::list \p List.
        /// \par Complexity
        /// \f$ O(1) \f$.
        /// \ingroup list
        template <typename List>
        using front = eval<detail::front_<List>>;

        namespace lazy
        {
            /// \sa 'meta::front'
            /// \ingroup lazy_list
            template <typename List>
            using front = defer<front, List>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // back
        /// \cond
        namespace detail
        {
            template <typename List>
            struct back_
            {
            };

            template <typename Head, typename... List>
            struct back_<list<Head, List...>>
            {
                using type = list_element_c<sizeof...(List), list<Head, List...>>;
            };
        } // namespace detail
        /// \endcond

        /// Return the last element in \c meta::list \p List.
        /// \par Complexity
        /// Amortized \f$ O(1) \f$.
        /// \ingroup list
        template <typename List>
        using back = eval<detail::back_<List>>;

        namespace lazy
        {
            /// \sa 'meta::back'
            /// \ingroup lazy_list
            template <typename List>
            using back = defer<back, List>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // push_front
        /// \cond
        namespace detail
        {
            template <typename List, typename T>
            struct push_front_
            {
            };

            template <typename... List, typename T>
            struct push_front_<list<List...>, T>
            {
                using type = list<T, List...>;
            };
        } // namespace detail
        /// \endcond

        /// Return a new \c meta::list by adding the element \c T to the front of \p List.
        /// \par Complexity
        /// \f$ O(1) \f$.
        /// \ingroup transformation
        template <typename List, typename T>
        using push_front = eval<detail::push_front_<List, T>>;

        namespace lazy
        {
            /// \sa 'meta::push_front'
            /// \ingroup lazy_transformation
            template <typename List, typename T>
            using push_front = defer<push_front, List, T>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // pop_front
        /// \cond
        namespace detail
        {
            template <typename List>
            struct pop_front_
            {
            };

            template <typename Head, typename... List>
            struct pop_front_<list<Head, List...>>
            {
                using type = list<List...>;
            };
        } // namespace detail
        /// \endcond

        /// Return a new \c meta::list by removing the first element from the front of \p List.
        /// \par Complexity
        /// \f$ O(1) \f$.
        /// \ingroup transformation
        template <typename List>
        using pop_front = eval<detail::pop_front_<List>>;

        namespace lazy
        {
            /// \sa 'meta::pop_front'
            /// \ingroup lazy_transformation
            template <typename List>
            using pop_front = defer<pop_front, List>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // push_back
        /// \cond
        namespace detail
        {
            template <typename List, typename T>
            struct push_back_
            {
            };

            template <typename... List, typename T>
            struct push_back_<list<List...>, T>
            {
                using type = list<List..., T>;
            };
        } // namespace detail
        /// \endcond

        /// Return a new \c meta::list by adding the element \c T to the back of \p List.
        /// \par Complexity
        /// \f$ O(1) \f$.
        /// \note pop_back not provided because it cannot be made to meet the
        /// complexity guarantees one would expect.
        /// \ingroup transformation
        template <typename List, typename T>
        using push_back = eval<detail::push_back_<List, T>>;

        namespace lazy
        {
            /// \sa 'meta::push_back'
            /// \ingroup lazy_transformation
            template <typename List, typename T>
            using push_back = defer<push_back, List, T>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // empty
        /// An Boolean integral constant wrapper around \c true if \p List is an
        /// empty type list; \c false, otherwise.
        /// \par Complexity
        /// \f$ O(1) \f$.
        /// \ingroup list
        template <typename List>
        using empty = bool_<0 == size<List>::type::value>;

        namespace lazy
        {
            /// \sa 'meta::empty'
            /// \ingroup lazy_list
            template <typename List>
            using empty = defer<empty, List>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // find
        /// \cond
        namespace detail
        {
            template <typename List, typename T>
            struct find_
            {
            };

            template <typename T>
            struct find_<list<>, T>
            {
                using type = list<>;
            };

            template <typename Head, typename... List, typename T>
            struct find_<list<Head, List...>, T> : find_<list<List...>, T>
            {
            };

            template <typename... List, typename T>
            struct find_<list<T, List...>, T>
            {
                using type = list<T, List...>;
            };
        } // namespace detail
        /// \endcond

        /// Return the tail of the list \p List starting at the first occurrence of
        /// \p T, if any such element exists; the empty list, otherwise.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup query
        template <typename List, typename T>
        using find = eval<detail::find_<List, T>>;

        namespace lazy
        {
            /// \sa 'meta::find'
            /// \ingroup lazy_query
            template <typename List, typename T>
            using find = defer<find, List, T>;
        }

        ////////////////////////////////////////////////////////////////////////////////////
        // reverse_find
        /// \cond
        namespace detail
        {
            template <typename List, typename T, typename State = list<>>
            struct reverse_find_
            {
            };

            template <typename T, typename State>
            struct reverse_find_<list<>, T, State>
            {
                using type = State;
            };

            template <typename Head, typename... List, typename T, typename State>
            struct reverse_find_<list<Head, List...>, T, State>
                : reverse_find_<list<List...>, T, State>
            {
            };

            template <typename... List, typename T, typename State>
            struct reverse_find_<list<T, List...>, T, State>
                : reverse_find_<list<List...>, T, list<T, List...>>
            {
            };
        }
        /// \endcond

        /// Return the tail of the list \p List starting at the last occurrence
        /// of \p T, if any such element exists; the empty list, otherwise.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup query
        template <typename List, typename T>
        using reverse_find = eval<detail::reverse_find_<List, T>>;

        namespace lazy
        {
            /// \sa 'meta::rfind'
            /// \ingroup lazy_query
            template <typename List, typename T>
            using reverse_find = defer<reverse_find, List, T>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // find_if
        /// \cond
        namespace detail
        {
            template <typename List, typename Fun>
            struct find_if_
            {
            };

            template <typename Fun>
            struct find_if_<list<>, Fun>
            {
                using type = list<>;
            };

            template <typename Head, typename... List, typename Fun>
            struct find_if_<list<Head, List...>, Fun>
                : if_<apply<Fun, Head>, id<list<Head, List...>>, find_if_<list<List...>, Fun>>
            {
            };
        } // namespace detail
        /// \endcond

        /// Return the tail of the list \p List starting at the first element `A`
        /// such that `apply<Fun, A>::%value` is \c true, if any such element
        /// exists; the empty list, otherwise.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup query
        template <typename List, typename Fun>
        using find_if = eval<detail::find_if_<List, Fun>>;

        namespace lazy
        {
            /// \sa 'meta::find_if'
            /// \ingroup lazy_query
            template <typename List, typename Fun>
            using find_if = defer<find_if, List, Fun>;
        }

        ////////////////////////////////////////////////////////////////////////////////////
        // reverse_find_if
        /// \cond
        namespace detail
        {
            template <typename List, typename Fun, typename State = list<>>
            struct reverse_find_if_
            {
            };

            template <typename Fun, typename State>
            struct reverse_find_if_<list<>, Fun, State>
            {
                using type = State;
            };

            template <typename Head, typename... List, typename Fun, typename State>
            struct reverse_find_if_<list<Head, List...>, Fun, State>
                : reverse_find_if_<list<List...>, Fun,
                                   if_<apply<Fun, Head>, list<Head, List...>, State>>
            {
            };
        }
        /// \endcond

        /// Return the tail of the list \p List starting at the last element `A` such that
        /// `apply<Fun, A>::%value` is \c true, if any such element exists; the empty list,
        /// otherwise.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup query
        template <typename List, typename Fun>
        using reverse_find_if = eval<detail::reverse_find_if_<List, Fun>>;

        namespace lazy
        {
            /// \sa 'meta::rfind_if'
            /// \ingroup lazy_query
            template <typename List, typename Fun>
            using reverse_find_if = defer<reverse_find_if, List, Fun>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // in
        /// A Boolean integral constant wrapper around \c true if there is at least one
        /// occurrence of \p T in \p List.
        /// \ingroup query
        template <typename List, typename T>
        using in = not_<empty<find<List, T>>>;

        namespace lazy
        {
            /// \sa 'meta::in'
            /// \ingroup lazy_query
            template <typename List, typename T>
            using in = defer<in, List, T>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // unique
        /// \cond
        namespace detail
        {
            template <typename List, typename Result>
            struct unique_
            {
            };

            template <typename Result>
            struct unique_<list<>, Result>
            {
                using type = Result;
            };

            template <typename Head, typename... List, typename Result>
            struct unique_<list<Head, List...>, Result>
                : unique_<list<List...>, apply<if_<in<Result, Head>, quote_trait<id>,
                                                   bind_back<quote<push_back>, Head>>,
                                               Result>>
            {
            };
        } // namespace detail
        /// \endcond

        /// Return a new \c meta::list where all duplicate elements have been removed.
        /// \par Complexity
        /// \f$ O(N^2) \f$.
        /// \ingroup transformation
        template <typename List>
        using unique = eval<detail::unique_<List, list<>>>;

        namespace lazy
        {
            /// \sa 'meta::unique'
            /// \ingroup lazy_transformation
            template <typename List>
            using unique = defer<unique, List>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // replace
        /// \cond
        namespace detail
        {
            template <typename List, typename T, typename U>
            struct replace_
            {
            };

            template <typename... List, typename T, typename U>
            struct replace_<list<List...>, T, U>
            {
                using type = list<if_<std::is_same<T, List>, U, List>...>;
            };
        } // namespace detail
        /// \endcond

        /// Return a new \c meta::list where all instances of type \p T have been replaced with
        /// \p U.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup transformation
        template <typename List, typename T, typename U>
        using replace = eval<detail::replace_<List, T, U>>;

        namespace lazy
        {
            /// \sa 'meta::replace'
            /// \ingroup lazy_transformation
            template <typename List, typename T, typename U>
            using replace = defer<replace, T, U>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // replace_if
        /// \cond
        namespace detail
        {
            template <typename List, typename C, typename U>
            struct replace_if_
            {
            };

            template <typename... List, typename C, typename U>
            struct replace_if_<list<List...>, C, U>
            {
                using type = list<if_<apply<C, List>, U, List>...>;
            };
        } // namespace detail
        /// \endcond

        /// Return a new \c meta::list where all elements \c A of the list \p List for which
        /// `apply<C,A>::%value` is \c true have been replaced with \p U.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup transformation
        template <typename List, typename C, typename U>
        using replace_if = eval<detail::replace_if_<List, C, U>>;

        namespace lazy
        {
            /// \sa 'meta::replace_if'
            /// \ingroup lazy_transformation
            template <typename List, typename C, typename U>
            using replace_if = defer<replace_if, C, U>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // fold
        /// \cond
        namespace detail
        {
            template <typename, typename, typename, typename = void>
            struct fold_
            {
            };

            template <typename State, typename Fun>
            struct fold_<list<>, State, Fun>
            {
                using type = State;
            };

            template <typename Head, typename... List, typename State, typename Fun>
            struct fold_<list<Head, List...>, State, Fun, void_<apply<Fun, State, Head>>>
                : fold_<list<List...>, apply<Fun, State, Head>, Fun>
            {
            };
        } // namespace detail
        /// \endcond

        /// Return a new \c meta::list constructed by doing a left fold of the list
        /// \p List using binary Metafunction Class \p Fun and initial state \p State.
        /// That is, the \c State_N for the list element \c A_N is computed by
        /// `Fun(State_N-1, A_N) -> State_N`.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup transformation
        template <typename List, typename State, typename Fun>
        using fold = eval<detail::fold_<List, State, Fun>>;

        /// An alias for `meta::fold`.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup transformation
        template <typename List, typename State, typename Fun>
        using accumulate = fold<List, State, Fun>;

        namespace lazy
        {
            /// \sa 'meta::foldl'
            /// \ingroup lazy_transformation
            template <typename List, typename State, typename Fun>
            using fold = defer<fold, List, State, Fun>;

            /// \sa 'meta::accumulate'
            /// \ingroup lazy_transformation
            template <typename List, typename State, typename Fun>
            using accumulate = defer<accumulate, List, State, Fun>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // reverse_fold
        /// \cond
        namespace detail
        {
            template <typename, typename, typename, typename = void>
            struct reverse_fold_
            {
            };

            template <typename State, typename Fun>
            struct reverse_fold_<list<>, State, Fun>
            {
                using type = State;
            };

            template <typename Head, typename... List, typename State, typename Fun>
            struct reverse_fold_<list<Head, List...>, State, Fun,
                                 void_<eval<reverse_fold_<list<List...>, State, Fun>>>>
                : lazy::apply<Fun, eval<reverse_fold_<list<List...>, State, Fun>>, Head>
            {
            };
        } // namespace detail
        /// \endcond

        /// Return a new \c meta::list constructed by doing a right fold of the list
        /// \p List using binary Metafunction Class \p Fun and initial state \p State.
        /// That is, the \c State_N for the list element \c A_N is computed by
        /// `Fun(A_N, State_N+1) -> State_N`.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup transformation
        template <typename List, typename State, typename Fun>
        using reverse_fold = eval<detail::reverse_fold_<List, State, Fun>>;

        namespace lazy
        {
            /// \sa 'meta::foldr'
            /// \ingroup lazy_transformation
            template <typename List, typename State, typename Fun>
            using reverse_fold = defer<reverse_fold, List, State, Fun>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // transform
        /// \cond
        namespace detail
        {
            template <typename, typename, typename = void>
            struct transform1_
            {
            };

            template <typename... List, typename Fun>
            struct transform1_<list<List...>, Fun, void_<list<apply<Fun, List>...>>>
            {
                using type = list<apply<Fun, List>...>;
            };

            template <typename, typename, typename, typename = void>
            struct transform2_
            {
            };

            template <typename... List0, typename... List1, typename Fun>
            struct transform2_<list<List0...>, list<List1...>, Fun,
                               void_<list<apply<Fun, List0, List1>...>>>
            {
                using type = list<apply<Fun, List0, List1>...>;
            };

            template <typename... Args>
            struct transform_
            {
            };

            template <typename List, typename Fun>
            struct transform_<List, Fun> : transform1_<List, Fun>
            {
            };

            template <typename List0, typename List1, typename Fun>
            struct transform_<List0, List1, Fun> : transform2_<List0, List1, Fun>
            {
            };
        } // namespace detail
        /// \endcond

        /// Return a new \c meta::list constructed by transforming all the elements in \p List
        /// with the unary Metafuncion Class \p Fun. \c transform can also be called with two
        /// lists of the same length and a binary Metafunction Class, in which case it returns
        /// a new list constructed with the results of calling \c Fun with each element in the
        /// lists, pairwise.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup transformation
        template <typename... Args>
        using transform = eval<detail::transform_<Args...>>;

        namespace lazy
        {
            /// \sa 'meta::transform'
            /// \ingroup lazy_transformation
            template <typename... Args>
            using transform = defer<transform, Args...>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // filter
        /// \cond
        namespace detail
        {
            template <typename Predicate>
            struct filter_
            {
                template <typename State, typename A>
                using apply =
                    meta::if_<meta::apply<Predicate, A>, meta::push_back<State, A>, State>;
            };
        } // namespace detail
        /// \endcond

        /// Returns a new meta::list where only those elements of \p List A that satisfy the
        /// Metafunction Class \p Predicate such that `apply<Pred,A>::%value` is \c true are
        /// present. That is, those elements that don't satisfy the \p Predicate are "removed".
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup transformation
        template <typename List, typename Predicate>
        using filter = meta::fold<List, meta::list<>, detail::filter_<Predicate>>;

        namespace lazy
        {
            /// \sa 'meta::filter'
            /// \ingroup lazy_transformation
            template <typename List, typename Predicate>
            using filter = defer<filter, List, Predicate>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // static_const
        ///\cond
        namespace detail
        {
            template <typename T>
            struct static_const
            {
                static constexpr T value{};
            };

            // Avoid potential ODR violations with global objects:
            template <typename T>
            constexpr T static_const<T>::value;
        } // namespace detail

        ///\endcond

        ///////////////////////////////////////////////////////////////////////////////////////////
        // for_each
        /// \cond
        namespace detail
        {
            struct for_each_fn
            {
                template <class UnaryFunction, class... Args>
                constexpr auto operator()(meta::list<Args...>, UnaryFunction f) const
                    -> UnaryFunction
                {
                    return (void)std::initializer_list<int>{(f(Args{}), void(), 0)...}, f;
                }
            };
        } // namespace detail
        /// \endcond

        ///\cond
        namespace
        {
            /// \endcond

            /// `for_each(List, UnaryFunction)` calls the \p UnaryFunction for each
            /// argument in the \p List.
            /// \ingroup runtime
            constexpr auto &&for_each = detail::static_const<detail::for_each_fn>::value;

            /// \cond
        }
        /// \endcond

        ///////////////////////////////////////////////////////////////////////////////////////////
        // zip_with
        /// Given a list of lists of types \p ListOfLists and a Metafunction Class \p Fun,
        /// construct a new list by calling \p Fun with the elements from the lists pairwise.
        /// \par Complexity
        /// \f$ O(N \times M) \f$, where \f$ N \f$ is the size of the outer list, and
        /// \f$ M \f$ is the size of the inner lists.
        /// \ingroup transformation
        template <typename Fun, typename ListOfLists>
        using zip_with = transform<fold<ListOfLists, repeat_n<size<front<ListOfLists>>, Fun>,
                                        bind_back<quote<transform>, quote<bind_front>>>,
                                   quote<apply>>;

        namespace lazy
        {
            /// \sa 'meta::zip_with'
            /// \ingroup lazy_transformation
            template <typename Fun, typename ListOfLists>
            using zip_with = defer<zip_with, Fun, ListOfLists>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // zip
        /// Given a list of lists of types \p ListOfLists, construct a new list by grouping the
        /// elements from the lists pairwise into `meta::list`s.
        /// \par Complexity
        /// \f$ O(N \times M) \f$, where \f$ N \f$ is the size of the outer list, and \f$ M \f$
        /// is the size of the inner lists.
        /// \ingroup transformation
        template <typename ListOfLists>
        using zip = zip_with<quote<list>, ListOfLists>;

        namespace lazy
        {
            /// \sa 'meta::zip'
            /// \ingroup lazy_transformation
            template <typename ListOfLists>
            using zip = defer<zip, ListOfLists>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // as_list
        /// \cond
        namespace detail
        {
            template <typename T>
            using uncvref_t = eval<std::remove_cv<eval<std::remove_reference<T>>>>;

            // Indirection here needed to avoid Core issue 1430
            // http://open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#1430
            template <typename Sequence>
            struct as_list_ : lazy::apply<uncurry<curry<quote_trait<id>>>, uncvref_t<Sequence>>
            {
            };
        } // namespace detail
        /// \endcond

        /// Turn a type into an instance of \c meta::list in a way determined by
        /// \c meta::apply_list.
        /// \ingroup list
        template <typename Sequence>
        using as_list = eval<detail::as_list_<Sequence>>;

        namespace lazy
        {
            /// \sa 'meta::as_list'
            /// \ingroup lazy_list
            template <typename Sequence>
            using as_list = defer<as_list, Sequence>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // reverse
        /// Return a new \c meta::list by reversing the elements in the list \p List.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup transformation
        template <typename List>
        using reverse = reverse_fold<List, list<>, quote<push_back>>;

        namespace lazy
        {
            /// \sa 'meta::reverse'
            /// \ingroup lazy_transformation
            template <typename List>
            using reverse = defer<reverse, List>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // all_of
        /// A Boolean integral constant wrapper around \c true if `apply<F, A>::%value`
        /// is \c true for all elements \c A in \c meta::list \p List; \c false, otherwise.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup query
        template <typename List, typename F>
        using all_of = empty<find_if<List, compose<quote<not_>, F>>>;

        namespace lazy
        {
            /// \sa 'meta::all_of'
            /// \ingroup lazy_query
            template <typename List, typename Fn>
            using all_of = defer<all_of, List, Fn>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // any_of
        /// A Boolean integral constant wrapper around \c true if `apply<F, A>::%value` is
        /// \c true for any element \c A in \c meta::list \p List; \c false, otherwise.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup query
        template <typename List, typename F>
        using any_of = not_<empty<find_if<List, F>>>;

        namespace lazy
        {
            /// \sa 'meta::any_of'
            /// \ingroup lazy_query
            template <typename List, typename Fn>
            using any_of = defer<any_of, List, Fn>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // none_of
        /// A Boolean integral constant wrapper around \c true if `apply<F, A>::%value` is
        /// \c false for all elements \c A in \c meta::list \p List; \c false, otherwise.
        /// \par Complexity
        /// \f$ O(N) \f$.
        /// \ingroup query
        template <typename List, typename F>
        using none_of = empty<find_if<List, F>>;

        namespace lazy
        {
            /// \sa 'meta::none_of'
            /// \ingroup lazy_query
            template <typename List, typename Fn>
            using none_of = defer<none_of, List, Fn>;
        }

        ////////////////////////////////////////////////////////////////////////////
        // lambda_
        /// \cond
        namespace detail
        {
            template <int, typename... As>
            struct lambda_
            {
            private:
                static constexpr std::size_t arity = sizeof...(As)-1;
                using Tags = list<As...>; // Includes the lambda body as the last arg!
                using F = back<Tags>;
                template <typename T, typename Args, typename Pos = reverse_find<Tags, T>>
                struct impl2
                {
                    using type = list_element_c<(Tags::size() - Pos::size()), Args>;
                };
                template <typename T, typename Args>
                struct impl2<T, Args, list<>>
                {
                    using type = T;
                };
                template <typename T, typename Args, typename = void>
                struct impl : impl2<T, Args>
                {
                };
                template <template <typename...> class C, typename... Ts, typename Args>
                struct impl<defer<C, Ts...>, Args, void_<C<eval<impl<Ts, Args>>...>>>
                {
                    using type = C<eval<impl<Ts, Args>>...>;
                };
                template <int N, typename... Ts, typename Args>
                struct impl<lambda_<N, Ts...>, Args>
                {
                    using type = impl;
                    template <typename... Us>
                    using apply = apply_list<lambda_<0, As..., Ts...>, concat<Args, list<Us...>>>;
                };
                template <template <typename...> class C, typename... Ts, typename Args>
                struct impl<C<Ts...>, Args, void_<C<eval<impl<Ts, Args>>...>>>
                {
                    using type = C<eval<impl<Ts, Args>>...>;
                };

            public:
                template <typename... Ts>
                using apply = eval<if_c<sizeof...(Ts) == arity, impl<F, list<Ts..., void>>>>;
            };
        }
        /// \endcond

        ///////////////////////////////////////////////////////////////////////////////////////////
        // lambda
        /// For creating anonymous Metafunction Classes.
        /// \code
        /// using L = lambda<_a, _b, std::pair<_b, std::pair<_a, _a>>>;
        /// using P = apply<L, int, short>;
        /// static_assert(std::is_same<P, std::pair<short, std::pair<int, int>>>::value, "");
        /// \endcode
        /// \ingroup metafunction
        template <typename... Ts>
        using lambda = detail::lambda_<0, Ts...>;

        // Some argument placeholders for use in \c lambda expressions.
        /// \ingroup metafunction
        inline namespace placeholders
        {
            struct _a;
            struct _b;
            struct _c;
            struct _d;
            struct _e;
            struct _f;
            struct _g;
            struct _h;
            struct _i;
        } // namespace placeholders

        ///////////////////////////////////////////////////////////////////////////////////////////
        // cartesian_product
        /// \cond
        namespace detail
        {
            template <typename M2, typename M>
            struct cartesian_product_fn
            {
                template <typename X>
                struct lambda0
                {
                    template <typename Xs>
                    using lambda1 = list<push_front<Xs, X>>;
                    using type = join<transform<M2, quote<lambda1>>>;
                };
                using type = join<transform<M, quote_trait<lambda0>>>;
            };
        } // namespace detail
        /// \endcond

        /// Given a list of lists \p ListOfLists, return a new list of lists that is
        /// the Cartesian Product. Like the `sequence` function from the Haskell Prelude.
        /// \par Complexity
        /// \f$ O(N \times M) \f$, where \f$ N \f$ is the size of the outer list, and
        /// \f$ M \f$ is the size of the inner lists.
        /// \ingroup transformation
        template <typename ListOfLists>
        using cartesian_product =
            reverse_fold<ListOfLists, list<list<>>, quote_trait<detail::cartesian_product_fn>>;

        namespace lazy
        {
            /// \sa 'meta::cartesian_product'
            /// \ingroup lazy_transformation
            template <typename ListOfLists>
            using cartesian_product = defer<cartesian_product, ListOfLists>;
        }

        /// \cond
        ///////////////////////////////////////////////////////////////////////////////////////////
        // add_const_if
        template <typename If>
        using add_const_if = if_<If, quote_trait<std::add_const>, quote_trait<id>>;

        template <bool If>
        using add_const_if_c = if_c<If, quote_trait<std::add_const>, quote_trait<id>>;
        /// \endcond

        ///////////////////////////////////////////////////////////////////////////////////////////
        // Math operations
        /// An integral constant wrapper around the result of adding the two wrapped integers
        /// \c T::type::value and \c U::type::value.
        /// \ingroup math
        template <typename T, typename U>
        using plus = std::integral_constant<decltype(T::type::value + U::type::value),
                                            T::type::value + U::type::value>;

        /// An integral constant wrapper around the result of subtracting the two wrapped integers
        /// \c T::type::value and \c U::type::value.
        /// \ingroup math
        template <typename T, typename U>
        using minus = std::integral_constant<decltype(T::type::value - U::type::value),
                                             T::type::value - U::type::value>;

        /// An integral constant wrapper around the result of multiplying the two wrapped integers
        /// \c T::type::value and \c U::type::value.
        /// \ingroup math
        template <typename T, typename U>
        using multiplies = std::integral_constant<decltype(T::type::value *U::type::value),
                                                  T::type::value * U::type::value>;

        /// An integral constant wrapper around the result of dividing the two wrapped integers \c
        /// T::type::value and \c U::type::value.
        /// \ingroup math
        template <typename T, typename U>
        using divides = std::integral_constant<decltype(T::type::value / U::type::value),
                                               T::type::value / U::type::value>;

        /// An integral constant wrapper around the remainder of dividing the two wrapped integers
        /// \c T::type::value and \c U::type::value.
        /// \ingroup math
        template <typename T>
        using negate = std::integral_constant<decltype(-T::type::value), -T::type::value>;

        /// An integral constant wrapper around the remainder of dividing the two wrapped integers
        /// \c T::type::value and \c U::type::value.
        /// \ingroup math
        template <typename T, typename U>
        using modulus = std::integral_constant<decltype(T::type::value % U::type::value),
                                               T::type::value % U::type::value>;

        /// A Boolean integral constant wrapper around the result of comparing \c T::type::value
        /// and \c U::type::value for equality.
        /// \ingroup math
        template <typename T, typename U>
        using equal_to = bool_<T::type::value == U::type::value>;

        /// A Boolean integral constant wrapper around the result of comparing \c T::type::value and
        /// \c U::type::value for inequality.
        /// \ingroup math
        template <typename T, typename U>
        using not_equal_to = bool_<T::type::value != U::type::value>;

        /// A Boolean integral constant wrapper around \c true if \c T::type::value is greater than
        /// \c U::type::value; \c false, otherwise.
        /// \ingroup math
        template <typename T, typename U>
        using greater = bool_<(T::type::value > U::type::value)>;

        /// A Boolean integral constant wrapper around \c true if \c T::type::value is less than \c
        /// U::type::value; \c false, otherwise.
        /// \ingroup math
        template <typename T, typename U>
        using less = bool_<(T::type::value < U::type::value)>;

        /// A Boolean integral constant wrapper around \c true if \c T::type::value is greater than
        /// or equal to \c U::type::value; \c false, otherwise.
        /// \ingroup math
        template <typename T, typename U>
        using greater_equal = bool_<(T::type::value >= U::type::value)>;

        /// A Boolean integral constant wrapper around \c true if \c T::type::value is less than or
        /// equal to \c U::type::value; \c false, otherwise.
        /// \ingroup math
        template <typename T, typename U>
        using less_equal = bool_<(T::type::value <= U::type::value)>;

        /// An integral constant wrapper around the result of bitwise-and'ing the two wrapped
        /// integers \c T::type::value and \c U::type::value.
        /// \ingroup math
        template <typename T, typename U>
        using bit_and = std::integral_constant<decltype(T::type::value &U::type::value),
                                               T::type::value & U::type::value>;

        /// An integral constant wrapper around the result of bitwise-or'ing the two wrapped
        /// integers \c T::type::value and \c U::type::value.
        /// \ingroup math
        template <typename T, typename U>
        using bit_or = std::integral_constant<decltype(T::type::value | U::type::value),
                                              T::type::value | U::type::value>;

        /// An integral constant wrapper around the result of bitwise-exclusive-or'ing the two
        /// wrapped integers \c T::type::value and \c U::type::value.
        /// \ingroup math
        template <typename T, typename U>
        using bit_xor = std::integral_constant<decltype(T::type::value ^ U::type::value),
                                               T::type::value ^ U::type::value>;

        /// An integral constant wrapper around the result of bitwise-complimenting the wrapped
        /// integer \c T::type::value.
        /// \ingroup math
        template <typename T>
        using bit_not = std::integral_constant<decltype(~T::type::value), ~T::type::value>;

        namespace lazy
        {
            /// \sa 'meta::plus'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using plus = defer<plus, T, U>;

            /// \sa 'meta::minus'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using minus = defer<minus, T, U>;

            /// \sa 'meta::multiplies'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using multiplies = defer<multiplies, T, U>;

            /// \sa 'meta::divides'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using divides = defer<divides, T, U>;

            /// \sa 'meta::negate'
            /// \ingroup lazy_math
            template <typename T>
            using negate = defer<negate, T>;

            /// \sa 'meta::modulus'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using modulus = defer<modulus, T, U>;

            /// \sa 'meta::equal_to'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using equal_to = defer<equal_to, T, U>;

            /// \sa 'meta::not_equal_t'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using not_equal_to = defer<not_equal_to, T, U>;

            /// \sa 'meta::greater'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using greater = defer<greater, T, U>;

            /// \sa 'meta::less'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using less = defer<less, T, U>;

            /// \sa 'meta::greater_equal'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using greater_equal = defer<greater_equal, T, U>;

            /// \sa 'meta::less_equal'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using less_equal = defer<less_equal, T, U>;

            /// \sa 'meta::bit_and'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using bit_and = defer<bit_and, T, U>;

            /// \sa 'meta::bit_or'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using bit_or = defer<bit_or, T, U>;

            /// \sa 'meta::bit_xor'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using bit_xor = defer<bit_xor, T, U>;

            /// \sa 'meta::bit_not'
            /// \ingroup lazy_math
            template <typename T>
            using bit_not = defer<bit_not, T>;
        }

        /// An integral constant wrapper around the minimum of \c T::type::value
        /// and \c U::type::value
        /// \ingroup math
        template <typename T, typename U>
        using min = meta::if_<meta::less<U, T>, U, T>;

        /// An integral constant wrapper around the maximum of \c T::type::value
        /// and \c U::type::value
        /// \ingroup math
        template <typename T, typename U>
        using max = meta::if_<meta::less<U, T>, T, U>;

        namespace lazy
        {
            /// \sa 'meta::max'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using max = defer<max, T, U>;

            /// \sa 'meta::min'
            /// \ingroup lazy_math
            template <typename T, typename U>
            using min = defer<min, T, U>;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        // integer_sequence

        /// A container for a sequence of compile-time integer constants.
        /// \ingroup integral
        template <typename T, T... Is>
        struct integer_sequence
        {
            using value_type = T;
            /// \return `sizeof...(Is)`
            static constexpr std::size_t size() noexcept { return sizeof...(Is); }
        };

        /// \cond
        namespace detail
        {
            // Glue two sets of integer_sequence together
            template <typename I1, typename I2, typename I3>
            struct integer_sequence_cat;

            template <typename T, T... N1, T... N2, T... N3>
            struct integer_sequence_cat<integer_sequence<T, N1...>, integer_sequence<T, N2...>,
                                        integer_sequence<T, N3...>>
            {
                using type = integer_sequence<T, N1..., (sizeof...(N1) + N2)...,
                                              (sizeof...(N1) + sizeof...(N2) + N3)...>;
            };

            template <typename T, std::size_t N>
            struct make_integer_sequence_
                : integer_sequence_cat<meta::eval<make_integer_sequence_<T, N / 2>>,
                                       meta::eval<make_integer_sequence_<T, N / 2>>,
                                       meta::eval<make_integer_sequence_<T, N % 2>>>
            {
            };

            template <typename T>
            struct make_integer_sequence_<T, 0>
            {
                using type = integer_sequence<T>;
            };

            template <typename T>
            struct make_integer_sequence_<T, 1>
            {
                using type = integer_sequence<T, 0>;
            };
        } // namespace detail
        /// \endcond

        /// Generate \c integer_sequence containing integer constants [0,1,2,...,N-1].
        /// \par Complexity
        /// \f$ O(log(N)) \f$.
        /// \ingroup integral
        template <typename T, T N>
        using make_integer_sequence = meta::eval<detail::make_integer_sequence_<T, (std::size_t)N>>;

        /// A container for a sequence of compile-time integer constants of type
        /// \c std::size_t
        /// \ingroup integral
        template <std::size_t... Is>
        using index_sequence = integer_sequence<std::size_t, Is...>;

        /// Generate \c index_sequence containing integer constants [0,1,2,...,N-1].
        /// \par Complexity
        /// \f$ O(log(N)) \f$.
        /// \ingroup integral
        template <std::size_t N>
        using make_index_sequence = make_integer_sequence<std::size_t, N>;

        /// \cond
    } // namespace v1
    /// \endcond
} // namespace meta

/// \cond
#if defined(__clang__) && defined(_LIBCPP_VERSION) && _LIBCPP_VERSION <= 1101

_LIBCPP_BEGIN_NAMESPACE_STD
template <class>
class _LIBCPP_TYPE_VIS_ONLY allocator;
template <class, class>
struct _LIBCPP_TYPE_VIS_ONLY pair;
template <class>
struct _LIBCPP_TYPE_VIS_ONLY hash;
template <class>
struct _LIBCPP_TYPE_VIS_ONLY less;
template <class>
struct _LIBCPP_TYPE_VIS_ONLY equal_to;
template <class>
struct _LIBCPP_TYPE_VIS_ONLY char_traits;
template <class, class>
class _LIBCPP_TYPE_VIS_ONLY list;
template <class, class>
class _LIBCPP_TYPE_VIS_ONLY forward_list;
template <class, class>
class _LIBCPP_TYPE_VIS_ONLY vector;
template <class, class>
class _LIBCPP_TYPE_VIS_ONLY deque;
template <class, class, class>
class _LIBCPP_TYPE_VIS_ONLY basic_string;
template <class, class, class, class>
class _LIBCPP_TYPE_VIS_ONLY map;
template <class, class, class, class>
class _LIBCPP_TYPE_VIS_ONLY multimap;
template <class, class, class>
class _LIBCPP_TYPE_VIS_ONLY set;
template <class, class, class>
class _LIBCPP_TYPE_VIS_ONLY multiset;
template <class, class, class, class, class>
class _LIBCPP_TYPE_VIS_ONLY unordered_map;
template <class, class, class, class, class>
class _LIBCPP_TYPE_VIS_ONLY unordered_multimap;
template <class, class, class, class>
class _LIBCPP_TYPE_VIS_ONLY unordered_set;
template <class, class, class, class>
class _LIBCPP_TYPE_VIS_ONLY unordered_multiset;
template <class, class>
class _LIBCPP_TYPE_VIS_ONLY queue;
template <class, class, class>
class _LIBCPP_TYPE_VIS_ONLY priority_queue;
template <class, class>
class _LIBCPP_TYPE_VIS_ONLY stack;
_LIBCPP_END_NAMESPACE_STD

namespace meta
{
    inline namespace v1
    {
        namespace detail
        {
            template <typename T, typename A = std::allocator<T>>
            using std_list = std::list<T, A>;
            template <typename T, typename A = std::allocator<T>>
            using std_forward_list = std::forward_list<T, A>;
            template <typename T, typename A = std::allocator<T>>
            using std_vector = std::vector<T, A>;
            template <typename T, typename A = std::allocator<T>>
            using std_deque = std::deque<T, A>;
            template <typename T, typename C = std::char_traits<T>, typename A = std::allocator<T>>
            using std_basic_string = std::basic_string<T, C, A>;
            template <typename K, typename V, typename C = std::less<K>,
                      typename A = std::allocator<std::pair<K const, V>>>
            using std_map = std::map<K, V, C, A>;
            template <typename K, typename V, typename C = std::less<K>,
                      typename A = std::allocator<std::pair<K const, V>>>
            using std_multimap = std::multimap<K, V, C, A>;
            template <typename K, typename C = std::less<K>, typename A = std::allocator<K>>
            using std_set = std::set<K, C, A>;
            template <typename K, typename C = std::less<K>, typename A = std::allocator<K>>
            using std_multiset = std::multiset<K, C, A>;
            template <typename K, typename V, typename H = std::hash<K>,
                      typename C = std::equal_to<K>,
                      typename A = std::allocator<std::pair<K const, V>>>
            using std_unordered_map = std::unordered_map<K, V, H, C, A>;
            template <typename K, typename V, typename H = std::hash<K>,
                      typename C = std::equal_to<K>,
                      typename A = std::allocator<std::pair<K const, V>>>
            using std_unordered_multimap = std::unordered_multimap<K, V, H, C, A>;
            template <typename K, typename H = std::hash<K>, typename C = std::equal_to<K>,
                      typename A = std::allocator<K>>
            using std_unordered_set = std::unordered_set<K, H, C, A>;
            template <typename K, typename H = std::hash<K>, typename C = std::equal_to<K>,
                      typename A = std::allocator<K>>
            using std_unordered_multiset = std::unordered_multiset<K, H, C, A>;
            template <typename T, typename C = std_deque<T>>
            using std_queue = std::queue<T, C>;
            template <typename T, typename C = std_vector<T>,
                      class D = std::less<typename C::value_type>>
            using std_priority_queue = std::priority_queue<T, C, D>;
            template <typename T, typename C = std_deque<T>>
            using std_stack = std::stack<T, C>;
        }

        template <>
        struct quote<::std::list> : quote<detail::std_list>
        {
        };
        template <>
        struct quote<::std::deque> : quote<detail::std_deque>
        {
        };
        template <>
        struct quote<::std::forward_list> : quote<detail::std_forward_list>
        {
        };
        template <>
        struct quote<::std::vector> : quote<detail::std_vector>
        {
        };
        template <>
        struct quote<::std::basic_string> : quote<detail::std_basic_string>
        {
        };
        template <>
        struct quote<::std::map> : quote<detail::std_map>
        {
        };
        template <>
        struct quote<::std::multimap> : quote<detail::std_multimap>
        {
        };
        template <>
        struct quote<::std::set> : quote<detail::std_set>
        {
        };
        template <>
        struct quote<::std::multiset> : quote<detail::std_multiset>
        {
        };
        template <>
        struct quote<::std::unordered_map> : quote<detail::std_unordered_map>
        {
        };
        template <>
        struct quote<::std::unordered_multimap> : quote<detail::std_unordered_multimap>
        {
        };
        template <>
        struct quote<::std::unordered_set> : quote<detail::std_unordered_set>
        {
        };
        template <>
        struct quote<::std::unordered_multiset> : quote<detail::std_unordered_multiset>
        {
        };
        template <>
        struct quote<::std::queue> : quote<detail::std_queue>
        {
        };
        template <>
        struct quote<::std::priority_queue> : quote<detail::std_priority_queue>
        {
        };
        template <>
        struct quote<::std::stack> : quote<detail::std_stack>
        {
        };
    }
}

#endif
/// \endcond

#endif
