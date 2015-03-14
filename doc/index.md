User Manual       {#mainpage}
===========

\tableofcontents

--------------------------------------------
*Meta* is a C++11 tiny metaprogramming library developed by
[Eric Niebler](https://github.com/ericniebler).

It is released under the Boost Software License and it is header only; that is,
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

The tutorial begins with a brief introduction to traits, first-class traits, and type
lists. Then it moves to trait composition and currying. Finally, it covers type list
algorithms and algorithms for working on integer sequences.

### Trait

Traits are compile-time functions that operate on types. For example,

\snippet example/tutorial_snippets.cpp trait0

is a trait taking an arbitrary number of types that always returns
`void`. The return value of a trait is obtained from a nested type alias
called (by convention) `type`. All of the traits in the C++11 standard
library follow this convention. C++11 template aliases allow us to write:

\snippet example/tutorial_snippets.cpp trait1

and thus to omit the `typename X::type` when calling a trait. C++14
standard library provides `_t` trait aliases for all the traits in
the standard library.

*Meta* provides `meta::eval<T>`, which evaluates the trait `T` by
 returning the tested `T::type` alias. This allows trait aliases to be
 written as follows:

\snippet example/tutorial_snippets.cpp trait2

### First-Class Trait

A *first-class trait* is a form of trait suitable for higher-order metaprogramming. It is
a class (not a template!) with a nested trait called (by convention) `apply`:

\snippet example/tutorial_snippets.cpp first_class_trait0

*Meta* provides the `meta::apply<F, Args...>` trait that evaluates the first-class trait
`F` with the arguments `Args`:

\snippet example/tutorial_snippets.cpp first_class_trait1

To turn an ordinary trait into a first-class trait *Meta* provides the
`meta::quote<F>` trait:

\snippet example/tutorial_snippets.cpp first_class_trait2

Note that in the first case we create a first-class trait that will evaluate the trait
through `apply`, while in the second case we create a first-class trait containing the
already evaluated result.

### Composition

Multiple first-class traits can be composed into a single first-class trait using
`meta::compose<F0, F1, ..., FN>`, which returns a new first-class trait that performs
`FN(... (F1(F0(Args...)) )`:

\snippet example/tutorial_snippets.cpp composition0

### Partial function application (currying)

Partial application is provided by binding a type to an argument of the trait using,
e.g., `meta::bind_front` and `meta::bind_back` to bind a type to the first/last
argument. That way, we can, e.g., create a trait that returns `true` if a type is the
same as `float` by reusing the `std::is_same` trait:

\snippet example/tutorial_snippets.cpp partial_application0

> TODO: `meta::curry` / `meta::uncurry`

### Logical operations

The traits `meta::if_`, `meta::and_`, `meta::or_`, and `meta::not_` cover
the basic logical operations with types:

\snippet example/tutorial_snippets.cpp logical_operations0

### Eager and lazy evaluation

> TODO trait aliases are eager, `meta::defer`, `meta::lazy` namespace.

### Lambdas

Lambda functions allow you to define traits in place:

\snippet example/tutorial_snippets.cpp lambda0

### Type lists

A list of types `Ts...` can be stored in the trait
`meta::list<Ts...>`. It provides a O(1) member function
`meta::list::size()` that returns the size of the list.

\snippet example/tutorial_snippets.cpp type_list0

As you can see, the `meta::front<List>`, `meta::back<List>`, and
`meta::at_c<List, std::size_t>` traits provide access to the
elements of the list. The `meta::empty<List>` trait returns `true` if
the list is empty. The `meta::at<List, meta::size_t<N>>` trait
differs from `meta::at_c` in that it takes a `meta::size_t<N>`
(`std::integral_constant<std::size_t, N>`) insted of an integer:

\snippet example/tutorial_snippets.cpp type_list1

You can add and remove elements from a list by using the transformation algorithms:

\snippet example/tutorial_snippets.cpp type_list2

You can concatenate, flatten, and zip multiple lists using
`meta::concat<Lists...>`, `meta::join<ListOfLists>`, and  `meta::zip<ListOfLists>`:

\snippet example/tutorial_snippets.cpp type_list3

> TODO:  `meta::zip_with` examples

Other typical operations on type lists include iteration, reductions, finding
elements, removing duplicates:

\snippet example/tutorial_snippets.cpp type_list4

To convert other type sequences into a `meta::list`, the utility trait
`meta::as_list<Sequence>` is provided. For example:

\snippet example/tutorial_snippets.cpp type_list5

To use meta with your own data types you can specialize the
`meta::extension::apply_list` trait for your own data type. For example,
to use meta with C++14 `std::integer_sequence`, you can:

\snippet example/tutorial_snippets.cpp type_list6

### Overview

This is a brief overview of the functionality in meta:

- Trait: `meta::eval`, `meta::apply`, `meta::defer`, `meta::quote`,
  `meta::quote_trait`, `meta::always`, `meta::id`,
  `meta::compose`, `meta::bind_front`, `meta::bind_back`, `meta::curry`,
  `meta::uncurry`, `meta::lambda`, `meta::let`, `meta::apply_list`.
- List: `meta::list`, `meta::front`, `meta::back`, `meta::at`,
  `meta::at_c`. `meta::empty`, `meta::size`.
- Logical: `meta::if_`, `meta::and_`, `meta::or_`, `meta::not_`.
- Query and search: `meta::all_of`, `meta::any_of`, `meta::none_of`, `meta::in`,
  `meta::find`, `meta::reverse_find`, `meta::find_if`, `meta::reverse_find_if`,
  `meta::cout`.
- Transformation: `meta::concat`, `meta::join`, `meta::zip`, `meta::zip_with`,
  `meta::as_list`, `meta::push_front`, `meta::push_back`, `meta::drop`,
  `meta::drop_c`, `meta::pop_front`, `meta::fold`, `meta::reverse_fold`,
  `meta::accumulate`, `meta::unique`, `meta::replace`, `meta::replace_if`,
  `meta::filter`, `meta::transform`, `meta::reverse`, `meta::cartesian_product`.
- Math: `meta::plus` , `meta::minus`, `meta::multiplies`, `meta::divides`,
  `meta::negate`, `meta::modulus`, `meta::equal_to`, `meta::not_equal_to`,
  `meta::greater`, `meta::less`, `meta::greater_equal`, `meta::less_equal`,
  `meta::bit_and`, `meta::bit_or`, `meta::bit_xor`, `meta::bit_not`,
  `meta::min`, `meta::max`, `meta::inc`, `meta::dec`.
- Run time: `meta::for_each`.

See the reference section for more details.
