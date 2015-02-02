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

### Type lists

A list of types `Ts...` can be stored in the metafunction
`meta::list<Ts...>`. It provides a O(1) member function
`meta::list::size()` that returns the size of the list.

You can concatenate multiple lists using `meta::concat<Lists...>`:

\snippet example/tutorial_snippets.cpp type_list0

To flatten a list of lists, `meta::join<ListOfLists>` is provided:

\snippet example/tutorial_snippets.cpp type_list1

To convert other type sequences into a `meta::List`, the utility metafunction
`meta::as_list<Sequence>` is provided. For example:

\snippet example/tutorial_snippets.cpp type_list2

> TODO: specify how to extend as_list to work on custom type sequences

### Composition

Multiple metafunctions classes can be composed into a single metafunction using
`meta::compose<F0, F1, ..., FN>`, which returns a new metafunction class that
performs `FN(... (F1(F0(Args...)) )`:

\snippet example/tutorial_snippets.cpp composition0

### Currying
