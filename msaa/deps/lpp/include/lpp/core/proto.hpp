#pragma once

#include <lpp/core/basic.hpp>

namespace lpp {
    namespace proto {

        /* is_implemented
            - Check whether a protocol is implemented.
            - Eg: is_implemented<Some_Protocol<S32, 42, Foo>>().
        */
        template <typename Proto>
        constexpr Bool is_implemented() {
            return meta::is_complete<Proto>();
        }

    }
}

/* LPP_DEF_PROTO
    - Define a new protocol.
    - `params` are the template parameters of the protocol.
      eg: LPP_PASS(typename T, Usize n)
    - `args` are those same parameters without the type.
      eg: LPP_PASS(T, n)
*/
#define LPP_DEF_PROTO(name, params, args)                                       \
    template <params>                                                           \
    struct LPP_CONCAT(Impl_, name);                                             \
    \
    template <params, Bool _is_implemented = ::lpp::proto::is_implemented<LPP_CONCAT(Impl_, name)<args>>()> \
    struct name {                                                               \
        static constexpr Bool is_implemented = false;                           \
        \
        static Ptr<LPP_CONCAT(name, _Record)> get() {                           \
            static_assert(false, #name " is not implemented.");                 \
        }                                                                       \
        \
        static Opt_Ptr<LPP_CONCAT(name, _Record)> get_maybe() {                 \
            return nullptr;                                                     \
        }                                                                       \
    };                                                                          \
    \
    template <params>                                                           \
    struct name<args, true> {                                                   \
        static constexpr Bool is_implemented = true;                            \
        \
        static Ptr<LPP_CONCAT(name, _Record)> get() {                           \
            return LPP_CONCAT(Impl_, name)<args>::get();                        \
        }                                                                       \
        \
        static Opt_Ptr<LPP_CONCAT(name, _Record)> get_maybe() {                 \
            return LPP_CONCAT(Impl_, name)<args>::get();                        \
        }                                                                       \
    }

#define LPP_IMPL_PROTO(name, params, args, code)                                \
    template <params>                                                           \
    struct LPP_CONCAT(Impl_, name)<args> {                                      \
        static Ptr<LPP_CONCAT(name, _Record)> get() {                           \
            static LPP_CONCAT(name, _Record) record = {                         \
                code                                                            \
            };                                                                  \
            return &record;                                                     \
        }                                                                       \
    }

