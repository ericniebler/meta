# Meta: A tiny metaprogramming library

[![Build Status](https://travis-ci.org/ericniebler/meta.svg?branch=master)](https://travis-ci.org/ericniebler/meta)

*Meta* is a tiny and header-only C++11 metaprogramming library released under the
Boost Software License. Supported compilers are clang >= 3.4 and gcc >= 4.9. To compile with meta you just have to:

```.cpp
#include <meta/meta.hpp>
```

You can find documentation online [here](https://ericniebler.github.io/meta/index.html).

For a quick start see Eric Niebler's blog post:
[A tiny metaprogramming library](http://ericniebler.com/2014/11/13/tiny-metaprogramming-library/). (Note: the names in Meta are different from those describe in the blog post, but the overall design remains the same.)

To generate the up-to-date tutorial and documentation run `make doc` in the
build directory (requires Doxygen, LaTeX, dvips, ghostscript).
