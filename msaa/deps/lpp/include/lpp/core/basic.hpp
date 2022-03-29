#pragma once


// SECTION: MSVC config.
#ifdef _MSC_VER

    #pragma warning(disable: 4456) // shadow locals
    #pragma warning(disable: 4457) // shadow parameters
    #pragma warning(disable: 4458) // shadow class members
    #pragma warning(disable: 4459) // shadow globals
    #pragma warning(disable: 4201) // unnamed struct
    #pragma warning(disable: 4820) // padding
    #pragma warning(disable: 4324) // padding
    #pragma warning(disable: 5045) // spectre mitigation info
    #pragma warning(disable: 4710) // inline was ignored
    #pragma warning(disable: 4711) // inline was performed
    #pragma warning(disable: 4514) // unused inline function removed
    #pragma warning(disable: 4204) // non-constant initializers
    #pragma warning(disable: 4668) // allow #if x, x undefined (replaced by 0)

    #ifndef LPP_DEBUG
        #ifdef _DEBUG
            #define LPP_DEBUG 1
        #else
            #define LPP_DEBUG 0
        #endif
    #endif

    #ifdef _WINDLL
        #define LPP_API __declspec(dllexport)
    #elif !defined(LPP_STATIC)
        #define LPP_API __declspec(dllimport)
    #else
        #define LPP_API
    #endif

#else

    #define LPP_API

#endif


#define NOMINMAX
#include <cstdint>
#include <cstddef>

namespace lpp {

    // SECTION: common macros.

    #define LPP_UNUSED(...) ((void)__VA_ARGS__)

    #define _LPP_STRINGIFY(x) #x
    #define LPP_STRINGIFY(x) _LPP_STRINGIFY(x)

    #define _LPP_CONCAT(a, b) a##b
    #define LPP_CONCAT(a, b) _LPP_CONCAT(a, b)

    #define LPP_PASS(...) __VA_ARGS__


    #define LPP_DEF_MOVE_CTOR(name, name_with_args) \
        name(RRef<name_with_args> other) { move(this, &other); } \
        Ref<name_with_args> operator=(RRef<name_with_args> other) { move(this, &other); return *this; }

    #define LPP_NO_COPY_CTOR(name, name_with_args) \
        name(Ref<name_with_args> other) = delete; \
        Ref<name_with_args> operator=(Ref<name_with_args>) = delete

    #define LPP_MOVE_IS_DESTROY_CTORS(name, name_with_args) \
        LPP_DEF_MOVE_CTOR(LPP_PASS(name), LPP_PASS(name_with_args)) \
        LPP_NO_COPY_CTOR (LPP_PASS(name), LPP_PASS(name_with_args))


    // SECTION: basic types.

    using Void = void;
    using Bool = bool;
    using Byte = unsigned char;

    using U8    = uint8_t;
    using U16   = uint16_t;
    using U32   = uint32_t;
    using U64   = uint64_t;
    using Usize = uintptr_t;

    using S8    = int8_t;
    using S16   = int16_t;
    using S32   = int32_t;
    using S64   = int64_t;
    using Ssize = intptr_t;

    using F32 = float;
    using F64 = double;


    constexpr Bool _not(Bool value) { return !value; }


    template <typename T> using Ptr  = T*;
    template <typename T> using Ref  = T&;
    template <typename T> using RRef = T&&;

    using Addr = Ptr<Byte>;


    // SECTION: encoded optional types.

    template <typename T, T none_value>
    struct Encoded_Option {
        T value;

        Encoded_Option() : value(none_value) {}
        Encoded_Option(T value) : value(value) {}
        Encoded_Option(Ref<const Encoded_Option<T, none_value>> other) : value(other.value) {}

        Bool is_some() const { return this->value != none_value; }
        Bool is_none() const { return this->value == none_value; }

        Ref<T> unwrap() {
            if(this->is_none()) { throw "Option was none."; }
            return this->value;
        }

        T unwrap_or(T other) {
            return this->is_some() ? this->value : other;
        }
    };


    template <typename T>
    using Opt_Ptr  = Encoded_Option<Ptr<T>, Ptr<T>(nullptr)>;
    using Opt_Addr = Opt_Ptr<Byte>;


    template <typename T> constexpr T get_max_value();
    template <> constexpr U8  get_max_value<U8 >() { return U8 (-1); }
    template <> constexpr U16 get_max_value<U16>() { return U16(-1); }
    template <> constexpr U32 get_max_value<U32>() { return U32(-1); }
    template <> constexpr U64 get_max_value<U64>() { return U64(-1); }

    template <typename T>
    using Max_Opt = Encoded_Option<T, get_max_value<T>()>;




    // SECTION: template meta utilities.

    namespace meta {
        struct True {
            constexpr static Bool value = true;
        };

        struct False {
            constexpr static Bool value = false;
        };


        template <typename T, Usize = sizeof(T)>
        True  _is_complete(Ptr<T>);
        False _is_complete(...);

        template <typename T>
        constexpr Bool is_complete() {
            return decltype(_is_complete(Ptr<T>()))::value;
        }

        template <typename T> struct _Remove_Reference          { using type = T; };
        template <typename T> struct _Remove_Reference<Ref<T>>  { using type = T; };
        template <typename T> struct _Remove_Reference<RRef<T>> { using type = T; };
        template <typename T>
        using Remove_Reference = typename _Remove_Reference<T>::type;


        template <typename T, typename U>
        struct Types_Are_Equal : False {};
        template <typename T>
        struct Types_Are_Equal<T, T> : True {};

        template <typename T, typename U>
        constexpr Bool are_equal() {
            return Types_Are_Equal<T, U>::value;
        }
    }



    // SECTION: std::move/forward replacements.

    #define lpp_move(...)    (static_cast<::lpp::meta::Remove_Reference<decltype(__VA_ARGS__)>&&>(__VA_ARGS__))
    #define lpp_forward(...) (static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__))



    // SECTION: defer.

    template <typename F>
    struct _Defer_Executor {
        F f;
        _Defer_Executor(F&& f) : f(lpp_forward(f)) {}
        ~_Defer_Executor() { f(); }
        Ref<_Defer_Executor> operator=(Ref<const _Defer_Executor>) = delete;
    };

    struct _Defer_Maker {
        template<typename F>
        _Defer_Executor<F> operator+(F&& f) { return _Defer_Executor<F>(lpp_forward(f)); }
    };

    #define lpp_defer auto LPP_CONCAT(__lpp_defer_, __LINE__) = ::lpp::_Defer_Maker {} + [&]()



    // SECTION: byte utilities.

    LPP_API Void copy_bytes(Addr dst, Addr src, Usize count);
    LPP_API Void set_bytes(Addr dst, Byte value, Usize count);
    LPP_API Void set_bytes_array(Addr base, Byte value, Usize size, Usize begin, Usize end);
    LPP_API Bool bytes_equal(Addr a, Addr b, Usize count);

    LPP_API Bool are_overlapping(Addr a, Addr b, Usize size);


    constexpr Byte debug_clear_value = 0xf0;

    LPP_API Void debug_clear(Addr object, Usize size);
    LPP_API Void debug_clear_array(Addr base, Usize size, Usize begin, Usize end);

    #if LPP_DEBUG == 0
    inline Void debug_clear(Addr, Usize) {}
    inline Void debug_clear_array(Addr, Usize, Usize, Usize) {}
    #endif

}

