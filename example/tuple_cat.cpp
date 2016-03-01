/// \file tuple_cat.cpp Example implementation of tuple cat using meta
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
/// Acknowledgements: Thanks Stephan T. Lavavej for spreading the word about
///                   tuple_cat, and the idea of bundling the tuples into a
///                   tuple of tuples and then using 2-dimensional indexing.
///                   He also discovered a bug in the implementation when
///                   using tuple_cat to concatenate temporary tuples
///                   including move-only types.
#include <tuple>
#include <memory>
#include <meta/meta.hpp>

using namespace meta;

// Tuple cat is a function that takes N tuples and glues them together into one.
//
// The main idea behind the implementation is to take all the tuples and build
// them up into a tuple of tuples. Since tuples are random-access, we can index
// into this 2-dimensional array of tuples with (i,j) coordinates, where i is
// the tuple index, and j the index of an element within the tuple i.

// This helper function takes a tuple of tuples (Tuples), a list of tuple
// indices (Is), a list of tuple element indices (Js), and the type of the
// tuple to be returned.
template <typename Ret, typename... Is, typename... Js, typename Tuples>
Ret tuple_cat_helper(list<Is...>, list<Js...>, Tuples &&tpls)
{
    // Note that for each element in a tuple we have a coordinate pair (i, j),
    // that is, the length of the lists containing the Is and the Js must be
    // equal:
    static_assert(sizeof...(Is) == sizeof...(Js), "");
    // It then explodes the tuple of tuples into the return type using the
    // coordinates (i, j) for each element:
    return Ret{std::get<Js::value>(std::get<Is::value>(std::forward<Tuples>(tpls)))...};
}

// Now we can implement tuple cat as follows:
// - first we compute the inner indices (the i's), that is, to which tuple each
//   element belongs,
// - then we compute the outer indices (the j's), that is, the position of each
//   element within their respective tuple
template <typename... Tuples,
          // The return type of the tuple is computed by: creating a list for the
          // elements of each tuple: list<...>, list<...>, ... Concatenating the lists
          // into a single list, and returning a tuple of the list elements.
          typename Res = apply<quote<std::tuple>, concat<as_list<Tuples>...>>>
Res tuple_cat(Tuples &&... tpls)
{
    // With sizeof we compute the # of tuples to concatenate:
    static constexpr std::size_t N = sizeof...(Tuples);

    // To compute the inner indices:
    //
    // - First, we make a list containing one list
    // with the elements of each tuple:
    using list_of_lists_of_tuple_elements = list<as_list<Tuples>...>;
    //   that is, for 3 tuples:
    //     - tuple0<int, float>,
    //     - tuple1<char>, and
    //     - tuple2<float, unsigned>,
    //   we have: list<list<int, float>, list<char>, list<float, unsigned>.
    //
    // - Then, we create for each tuple, a Callable that, when called with
    // any type, returns the tuple index. That is, we make an index sequence
    // from [0, N), e.g., [0, 1, 2], and transform it with the Callable
    // id, such that we get a list of Callables:
    // [always0, always1, always2].
    using always_tuple_index = transform<as_list<make_index_sequence<N>>, quote<id>>;
    //
    // - Afterwards, we transform(list_of_tuples, always_tuple_indices,
    // transform):
    //   That is, for each list of tuple elements, e.g., list<int, float>,
    //   we call transform again with a Callable, that no matter what type
    //   is passed, returns the tuple index:
    //   transform(list<int, float>, always0)
    //       -> list<always0(int), always0(float)>
    //       -> list<0, 0>
    //   For the next tuples we get a list<1>, and list<2, 2>.
    //   This returns a list of lists of inner indices:
    using list_of_list_of_inner_indices =
        transform<list_of_lists_of_tuple_elements, always_tuple_index, quote<transform>>;
    //   That is, listlist<0, 0>, list<1>, list<2,2>>
    //
    // - Finally: we flatten the list, and get the list of inner indices:
    using inner = join<list_of_list_of_inner_indices>; // e.g. [0, 0, 1, 2, 2]

    // To compute the outer indices:
    //
    // - Create a list of lists of tuple elements as above
    //
    // - For each lists, replace the list with a list of [0, size(list)) for
    //   each list. This is done by calling transform on the list of lists of
    //   tuple elements, with a Callable that gets the list size, makes an
    //   index sequence, and returns that as a list:
    //     f(list) = as_list(make_index_sequence(size(list)));
    //   This produces list<list<0, 1>, list<0>, list<0, 1>>:
    // - Finally, we flatten the result: list<0, 1, 0, 0, 1>.
    using outer = join< // flatten:
        // replace list of list of tuple elements, with list of indices for each
        // tuple:
        transform<list<as_list<Tuples>...>,
                  // f(list) = as_list(make_index_sequence(size(list)));
                  compose<quote<as_list>, quote_i<std::size_t, make_index_sequence>, quote<size>>>>;
    return tuple_cat_helper<Res>(inner{}, outer{},
                                 std::forward_as_tuple(std::forward<Tuples>(tpls)...));
}

int main()
{
    std::tuple<int, short, long> t1;
    std::tuple<> t2;
    std::tuple<float, double, long double> t3;
    std::tuple<void *, char *> t4;
    auto x = ::tuple_cat(t1, t2, t3, t4, std::make_tuple(std::unique_ptr<int>{}));
    using expected_type = std::tuple<int, short, long, float, double, long double, void *, char *,
                                     std::unique_ptr<int>>;
    static_assert(std::is_same<decltype(x), expected_type>::value, "");
}
