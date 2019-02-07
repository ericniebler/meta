User Manual       {#mainpage}
===========

\tableofcontents

--------------------------------------------
*Meta* is a C++11 tiny metaprogramming library developed by
[Eric Niebler](https://github.com/ericniebler) to facilitate the computation and
manipulation of types and lists of types (aka, variadic parameter packs).

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

## Quick Start

TODO show some simple uses. Make sure we show what Meta is good for before diving into
terminology and esoteric concepts.

## Tutorial

The tutorial begins with a brief introduction to traits, aliases, and callables.
Then it moves to trait composition and currying. Finally, it covers type list algorithms
and algorithms for working on integer sequences.

TODO This feels backwards. Algorithms come first. Everything else is in support of them.

### Traits

*Traits* are class templates that have a nested type alias called (by convention) `type`.
For example,

\snippet example/tutorial_snippets.cpp trait0

is a trait taking an arbitrary number of types that always "returns" `void`. There are
many familiar examples of traits in the Standard Library; `std::remove_reference` and
`std::decay` to name two.

## Aliases

An *alias* is a synonym for a type. C++11 introduced alias *templates*, which are names
that refer to a family of types. Alias templates simplify template syntax and smooth
out interface differences. Below is an example of an alias template:

\snippet example/tutorial_snippets.cpp trait1

Notice how `t_t<int, double>` becomes a synonym for `void`. The C++14 standard library
provides `_t` alias templates for all the traits in the standard library.

*Meta* provides `meta::_t<T>`, which evaluates the trait `T` by aliasing the nested
`T::type` alias. This allows us to alias the nested `type` of a trait as follows:

\snippet example/tutorial_snippets.cpp trait2

\note Alias templates have primacy in Meta. This is different from other metaprogramming
libraries you may be familiar with, which make traits (aka metafunctions) the prime
abstraction. The rest of this guide uses the term "alias" to mean "alias template".

### Callables

A *Callable* is a kind of alias suitable for higher-order metaprogramming. It is
a class (not a template!) with a nested alias called (by convention) `invoke`:

\snippet example/tutorial_snippets.cpp callable0

All of the algorithms that take "functions" as arguments expect Callables instead of
raw aliases.  *Meta* provides the `meta::invoke<F, Args...>` alias that evaluates the
Callable `F` with the arguments `Args`:

\snippet example/tutorial_snippets.cpp callable1

To turn an ordinary alias into a Callable *Meta* provides the `meta::quote<F>` trait:

\snippet example/tutorial_snippets.cpp callable2

Note that in the first case we create a Callable that evaluates to the trait itself,
while in the second case we create a Callable that evaluates to the nested `type` of
the trait.

When "quoting" a trait, it is often desirable for the resulting Callable to refer to
the nested `type` instead of the trait itself. For that we can use `meta::quote_trait`.
Consider:

\snippet example/tutorial_snippets.cpp callable3

Notice that `meta::quote<std::add_pointer_t>` and `meta::quote_trait<std::add_pointer>`
mean the same thing.

\note You may wonder what advantage Callables have over alias templates. A Callable is a
*type* that represents a computation. Much of Meta revolves around types and the
computation of types. Sometimes it's desirable to compute a computation, or to use a
computation as an argument to another computation. In those cases, it's very handy for
computations to themselves be types and not templates.

### Composition

Multiple Callables can be composed into a single Callable using
`meta::compose<F0, F1, ..., FN>`, which names a new Callable that performs
`F0(F1(...(FN(Args...))))`:

\snippet example/tutorial_snippets.cpp composition0

### Partial function application

You can turn a Callable expecting *N* arguments into a Callable expecting *N-M*
arguments by binding *M* arguments to the front or the back of its argument list. You can
use `meta::bind_front` and `meta::bind_back` for that. Below we create a Callable
that tests whether a type is `float` by reusing the `std::is_same` trait:

\snippet example/tutorial_snippets.cpp partial_application0

\note If `std::is_same` is a trait, why did we use `meta::quote` instead of
`meta::quote_trait`? In this case, it makes no difference. In addition to being a trait,
`std::is_same<X, Y>` inherits from `std::integral_constant<bool, true-or-false>` so we
can construct an instance of `std::is_same<X, Y>` and test it in a `constexr` Boolean
context.

### Logical operations

The traits `meta::if_`, `meta::and_`, `meta::or_`, and `meta::not_` cover
the basic logical operations with types:

\snippet example/tutorial_snippets.cpp logical_operations0

### Eager and lazy evaluation

> TODO aliases are eager, `meta::defer`, `meta::lazy` namespace.

### Lambdas

Lambda functions allow you to define Callables in place:

\snippet example/tutorial_snippets.cpp lambda0

### Type lists

A list of types `Ts...` can be stored in the type `meta::list<Ts...>`. It provides a O(1)
static member function `meta::list::size()` that returns the size of the list.

\snippet example/tutorial_snippets.cpp type_list0

As you can see, the `meta::front<List>`, `meta::back<List>`, and
`meta::at_c<List, std::size_t>` aliases provide access to the elements of the list. The
`meta::empty<List>` alias is `std::true_type` if the list is empty. The
`meta::at<List, meta::size_t<N>>` alias differs from `meta::at_c` in that it takes a
`meta::size_t<N>` (`std::integral_constant<std::size_t, N>`) instead of an integer:

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
`meta::extension::apply` trait for your own data type. For example,
to use meta with C++14 `std::integer_sequence`, you can:

\snippet example/tutorial_snippets.cpp type_list6

### Overview

This is a brief overview of the functionality in meta:

- Trait: `meta::_t`, `meta::_v`, `meta::invoke`, `meta::defer`,
  `meta::quote`, `meta::quote_trait`, `meta::id`,
  `meta::compose`, `meta::bind_front`, `meta::bind_back`, `meta::curry`,
  `meta::uncurry`, `meta::lambda`, `meta::let`, `meta::apply`.
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
