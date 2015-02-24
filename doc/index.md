User Manual       {#mainpage}
===========

\tableofcontents

--------------------------------------------
*Meta* is a C++14 tiny metaprogramming library developed by
[Eric Niebler](https://github.com/ericniebler).

It is released under the Boost Software License and it is header only, that is,
to compile with meta you just have to:

~~~~~~~{.cpp}
#include <meta/meta.hpp>
~~~~~~~

--------------------------------------------

The documentation of *Meta* is currently scarce. The best resources are the
<a href="group__meta.html">Reference</a> and the
<a href="examples.html">Examples</a>.

As a motivation and introduction to the library you can read
[Eric's original blog post](http://ericniebler.com/2014/11/13/tiny-metaprogramming-library/),
but please keep in mind that the library has evolved quite a bit since then.

--------------------------------------------

## Tutorial

The tutorial begins with a brief introduction to metafunctions, metafunction
classes, and type lists. Then it moves to metafunction composition and
currying. Finally, it covers type list algorithms and algorithms for working on
integer sequences.

### Metafunction

Metafunctions are functions that operate on types. For example,

\snippet example/tutorial_snippets.cpp meta_function0

is a metafunction taking an arbitrary number of types that always returns
void. The return value of a meta function is obtained from a nested type alias
called (by convention) `type`. All of the metafunctions in the C++11 standard
library follow this convention. C++11 template aliases allow us to write:

\snippet example/tutorial_snippets.cpp meta_function1

and thus to omit the `typename X::type` when calling a metafunction. C++14
standard library provides `_t` metafunction aliases for all the metafunctions in
the standard library.

*Meta* provides `meta::eval<T>`, which evaluates the metafunction `T` by
 returning the tested `T::type` alias. This allows metafunction aliases to be
 written as follows:

\snippet example/tutorial_snippets.cpp meta_function2

### Metafunction Class

A Metafunction Class is a form of metafunction suitable for higher-order
metaprogramming. It is a class with a nested metafunction called (by convention)
`apply`:

\snippet example/tutorial_snippets.cpp meta_function_class0

*Meta* provides the `meta::apply<F, Args...>` metafunction that evaluates the
 metafunction class `F` with the arguments `Args`:

\snippet example/tutorial_snippets.cpp meta_function_class1

To turn a metafunction into a metafunction class *Meta* provides the
`meta::quote<F>` metafunction:

\snippet example/tutorial_snippets.cpp meta_function_class2

Note that in the first case we create a metafunction class that will evaluate
the metafunction through apply, while in the second case we create a
metafunction class containing the already evaluated result.

### Composition

Multiple metafunctions classes can be composed into a single metafunction using
`meta::compose<F0, F1, ..., FN>`, which returns a new metafunction class that
performs `FN(... (F1(F0(Args...)) )`:

\snippet example/tutorial_snippets.cpp composition0

### Partial function application (currying)

Partial application is provided by binding a type to an argument of the
metafunction using, e.g., `meta::bind_front` and `meta::bind*back` to bind a
type to the first/last argument. That way, we can, e.g., create a metafunction
that returns \c true if a type is the same as `float` by reusing the `std::is_same`
metafunction:

\snippet example/tutorial_snippets.cpp partial_application0

> TODO: `meta::curry` / `meta::uncurry`

### Logical operations

The metafunctions `meta::if_`, `meta::and_`, `meta::or_`, and `meta::not_` cover
the basic logical operations with types:

\snippet example/tutorial_snippets.cpp logical_operations0

### Eager and lazy evaluation

> TODO meta function aliases are eager, `meta::defer`, `meta::lazy` namespace.

### Lambdas

Lambda functions allow you to define metafunctions in place:

\snippet example/tutorial_snippets.cpp lambda0


### Type lists

A list of types `Ts...` can be stored in the metafunction
`meta::list<Ts...>`. It provides a O(1) member function
`meta::list::size()` that returns the size of the list.

\snippet example/tutorial_snippets.cpp type_list0

As you can see, the `meta::front<List>`, `meta::back<List>`, and
`meta::list_element_c<std::size_t, List>` metafunctions provide access to the
elements of the list. The `meta::empty<List>` metafunction returns \c true if
the list is empty. The `meta::list_element<meta::size_t<N>, List>` metafunction
differs from `meta::list_element_c` in that it takes a `meta::size_t<N>`
(`std::integral_constant<std::size_t, N>`) insted of an integer:

\snippet example/tutorial_snippets.cpp type_list1

You can add and remove elements from a list by using the transformation algorithms:

\snippet example/tutorial_snippets.cpp type_list2

You can concatenate multiple lists using `meta::concat<Lists...>`:

\snippet example/tutorial_snippets.cpp type_list3

To flatten a list of lists, `meta::join<ListOfLists>` is provided:

\snippet example/tutorial_snippets.cpp type_list4

To zip a list of lists, `meta::zip<ListOfLists>` is provided:

\snippet example/tutorial_snippets.cpp type_list5

> TODO:  `meta::zip_with` examples

Other typical operations on type lists include iteration, reductions, finding
elements, removing duplicates:

\snippet example/tutorial_snippets.cpp type_list6

To convert other type sequences into a `meta::list`, the utility metafunction
`meta::as_list<Sequence>` is provided. For example:

\snippet example/tutorial_snippets.cpp type_list7

> TODO: specify how to extend `meta::as_list` to work on custom type sequences

### Overview

This is a brief overview of the functionality in meta:

- Metafunction: `meta::eval`, `meta::apply`, `meta::defer`, `meta::quote`,
  `meta::compose`, `meta::bind_front`, `meta::bind_back`, `meta::curry`,
  `meta::uncurry`, `meta::lambda`.
- List: `meta::list`, `meta::front`, `meta::back`, `meta::list_element`,
  `meta::list_element_c`. `meta::empty`, `meta::size`.
- Logical: `meta::if_`, `meta::and_`, `meta::or_`, `meta::not_`.
- Query and search: `meta::all_of`, `meta::any_of`, `meta::none_of`, `meta::in`,
  `meta::find`, `meta::rfind`, `meta::find_if`, `meta::rfind_if`.
- Transformation: `meta::concat`, `meta::join`, `meta::zip`, `meta::zip_with`,
  `meta::as_list`, `meta::push_front`, `meta::push_back`, `meta::drop`,
  `meta::drop_c`, `meta::pop_front`, `meta::foldl`, `meta::foldr`,
  `meta::accumulate`, `meta::unique`, `meta::replace`, `meta::replace_if`,
  `meta::filter`, `meta::transform`, `meta::reverse`, `meta::cartesian_product`.
- Math: `meta::plus` , `meta::minus`, `meta::multiplies`, `meta::divides`,
  `meta::negate`, `meta::modulus`, `meta::equal_to`, `meta::not_equal_to`,
  `meta::greater`, `meta::less`, `meta::greater_equal`, `meta::less_equal`,
  `meta::bit_and`, `meta::bit_or`, `meta::bit_xor`, `meta::bit_not`,
  `meta::min`, `meta::max`.
- Run time: `meta::for_each`.

See the reference section for more details.
