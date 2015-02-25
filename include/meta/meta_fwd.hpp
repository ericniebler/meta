/// \file meta_fwd.hpp Forward declarations
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

#ifndef META_FWD_HPP
#define META_FWD_HPP

namespace meta
{
    inline namespace v1
    {
        template <typename T, T...>
        struct integer_sequence;

        template <typename... Ts>
        struct list;

        template <typename T>
        struct id;

        template <template <typename...> class>
        struct quote;

        template <template <typename...> class C>
        struct quote_trait;

        template <typename T, template <T...> class F>
        struct quote_i;

        template <typename T, template <T...> class C>
        struct quote_trait_i;

        template <typename... Fs>
        struct compose;

        template <typename T>
        struct always;

        template <template <typename...> class C, typename... Ts>
        struct defer;

        namespace extension
        {
            template <typename F, typename List>
            struct apply_list;
        }

    } // inline namespace v1
} // namespace meta

#endif
