/// \file meta_libcxx_workaround.hpp Make meta::quote work consistently with
/// libc++ containers
///
/// Works around: http://llvm.org/bugs/show_bug.cgi?id=22601
///  and http://llvm.org/bugs/show_bug.cgi?id=22605 Meta library
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
#ifndef META_HPP_LIBCXX_WORKAROUND
#define META_HPP_LIBCXX_WORKAROUND

#if defined(__clang__) && defined(_LIBCPP_VERSION) && _LIBCPP_VERSION <= 1101

_LIBCPP_BEGIN_NAMESPACE_STD
template <class> class _LIBCPP_TYPE_VIS_ONLY allocator;
template <class, class> struct _LIBCPP_TYPE_VIS_ONLY pair;
template <class> struct _LIBCPP_TYPE_VIS_ONLY hash;
template <class> struct _LIBCPP_TYPE_VIS_ONLY less;
template <class> struct _LIBCPP_TYPE_VIS_ONLY equal_to;
template <class> struct _LIBCPP_TYPE_VIS_ONLY char_traits;
template <class, class> class _LIBCPP_TYPE_VIS_ONLY list;
template <class, class> class _LIBCPP_TYPE_VIS_ONLY forward_list;
template <class, class> class _LIBCPP_TYPE_VIS_ONLY vector;
template <class, class> class _LIBCPP_TYPE_VIS_ONLY deque;
template <class, class, class> class _LIBCPP_TYPE_VIS_ONLY basic_string;
template <class, class, class, class> class _LIBCPP_TYPE_VIS_ONLY map;
template <class, class, class, class> class _LIBCPP_TYPE_VIS_ONLY multimap;
template <class, class, class> class _LIBCPP_TYPE_VIS_ONLY set;
template <class, class, class> class _LIBCPP_TYPE_VIS_ONLY multiset;
template <class, class, class, class, class>
class _LIBCPP_TYPE_VIS_ONLY unordered_map;
template <class, class, class, class, class>
class _LIBCPP_TYPE_VIS_ONLY unordered_multimap;
template <class, class, class, class> class _LIBCPP_TYPE_VIS_ONLY unordered_set;
template <class, class, class, class>
class _LIBCPP_TYPE_VIS_ONLY unordered_multiset;
template <class, class> class _LIBCPP_TYPE_VIS_ONLY queue;
template <class, class, class> class _LIBCPP_TYPE_VIS_ONLY priority_queue;
template <class, class> class _LIBCPP_TYPE_VIS_ONLY stack;
_LIBCPP_END_NAMESPACE_STD

namespace meta
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
        template <typename T, typename C = std::char_traits<T>,
                  typename A = std::allocator<T>>
        using std_basic_string = std::basic_string<T, C, A>;
        template <typename K, typename V, typename C = std::less<K>,
                  typename A = std::allocator<std::pair<K const, V>>>
        using std_map = std::map<K, V, C, A>;
        template <typename K, typename V, typename C = std::less<K>,
                  typename A = std::allocator<std::pair<K const, V>>>
        using std_multimap = std::multimap<K, V, C, A>;
        template <typename K, typename C = std::less<K>,
                  typename A = std::allocator<K>>
        using std_set = std::set<K, C, A>;
        template <typename K, typename C = std::less<K>,
                  typename A = std::allocator<K>>
        using std_multiset = std::multiset<K, C, A>;
        template <typename K, typename V, typename H = std::hash<K>,
                  typename C = std::equal_to<K>,
                  typename A = std::allocator<std::pair<K const, V>>>
        using std_unordered_map = std::unordered_map<K, V, H, C, A>;
        template <typename K, typename V, typename H = std::hash<K>,
                  typename C = std::equal_to<K>,
                  typename A = std::allocator<std::pair<K const, V>>>
        using std_unordered_multimap = std::unordered_multimap<K, V, H, C, A>;
        template <typename K, typename H = std::hash<K>,
                  typename C = std::equal_to<K>, typename A = std::allocator<K>>
        using std_unordered_set = std::unordered_set<K, H, C, A>;
        template <typename K, typename H = std::hash<K>,
                  typename C = std::equal_to<K>, typename A = std::allocator<K>>
        using std_unordered_multiset = std::unordered_multiset<K, H, C, A>;
        template <typename T, typename C = std_deque<T>>
        using std_queue = std::queue<T, C>;
        template <typename T, typename C = std_vector<T>,
                  class D = std::less<typename C::value_type>>
        using std_priority_queue = std::priority_queue<T, C, D>;
        template <typename T, typename C = std_deque<T>>
        using std_stack = std::stack<T, C>;
    }

    template <> struct quote< ::std::list > : quote<detail::std_list>
    {
    };
    template <> struct quote< ::std::deque > : quote<detail::std_deque>
    {
    };
    template <>
    struct quote< ::std::forward_list > : quote<detail::std_forward_list>
    {
    };
    template <> struct quote< ::std::vector > : quote<detail::std_vector>
    {
    };
    template <>
    struct quote< ::std::basic_string > : quote<detail::std_basic_string>
    {
    };
    template <> struct quote< ::std::map > : quote<detail::std_map>
    {
    };
    template <> struct quote< ::std::multimap > : quote<detail::std_multimap>
    {
    };
    template <> struct quote< ::std::set > : quote<detail::std_set>
    {
    };
    template <> struct quote< ::std::multiset > : quote<detail::std_multiset>
    {
    };
    template <>
    struct quote< ::std::unordered_map > : quote<detail::std_unordered_map>
    {
    };
    template <>
    struct quote< ::std::unordered_multimap >
      : quote<detail::std_unordered_multimap>
    {
    };
    template <>
    struct quote< ::std::unordered_set > : quote<detail::std_unordered_set>
    {
    };
    template <>
    struct quote< ::std::unordered_multiset >
      : quote<detail::std_unordered_multiset>
    {
    };
    template <> struct quote< ::std::queue > : quote<detail::std_queue>
    {
    };
    template <>
    struct quote< ::std::priority_queue > : quote<detail::std_priority_queue>
    {
    };
    template <> struct quote< ::std::stack > : quote<detail::std_stack>
    {
    };
}

#endif

#endif
