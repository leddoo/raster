#pragma once

#include <lpp/core/basic.hpp>


namespace lpp {

    template <typename T, Usize n>
    struct Vector : Array<T, n> {
        using Array<T, n>::Array;

        Vector() {}

        explicit Vector(T value) {
            for(auto& entry : *this) {
                entry = value;
            }
        }


        #define lpp_def_vector_access_element(name, index) \
            Ref<T>       name()       { static_assert(index < n, "Index out of bounds."); return this->values()[index]; } \
            Ref<const T> name() const { static_assert(index < n, "Index out of bounds."); return this->values()[index]; }


        lpp_def_vector_access_element(x, 0);
        lpp_def_vector_access_element(r, 0);
        lpp_def_vector_access_element(y, 1);
        lpp_def_vector_access_element(g, 1);
        lpp_def_vector_access_element(z, 2);
        lpp_def_vector_access_element(b, 2);
        lpp_def_vector_access_element(w, 3);
        lpp_def_vector_access_element(a, 3);

    };


    template <typename T> using V1 = Vector<T, 1>;
    template <typename T> using V2 = Vector<T, 2>;
    template <typename T> using V3 = Vector<T, 3>;
    template <typename T> using V4 = Vector<T, 4>;

    using V1f = V1<F32>;
    using V2f = V2<F32>;
    using V3f = V3<F32>;
    using V4f = V4<F32>;

    using V1s = V1<S32>;
    using V2s = V2<S32>;
    using V3s = V3<S32>;
    using V4s = V4<S32>;

    using V1u = V1<U32>;
    using V2u = V2<U32>;
    using V3u = V3<U32>;
    using V4u = V4<U32>;


    /* about these macros:
        Sadly the first version is not enough, as msvc gets very confused by
        the loop. It can't get rid of it and produces code that is up to 6x
        slower than with the extra "unrolled" definitions.
        Using Ref<const> costs another 20%, so we pass by value.
    */

    #define lpp_def_vector_proc(_template, name, params, expr)                  \
        _template                                                               \
        Vector<T, n> name(params) {                                             \
            Vector<T, n> result;                                                \
            for(auto i : Range<Usize>(n)) {                                     \
                result[i] = (expr);                                             \
            }                                                                   \
            return result;                                                      \
        }

    #define lpp_def_vector1_proc(_template, name, params, expr)                 \
        _template                                                               \
        Vector<T, 1> name(params) {                                             \
            Vector<T, 1> result;                                                \
            { constexpr Usize i = 0; result[i] = (expr); }                      \
            return result;                                                      \
        }

    #define lpp_def_vector2_proc(_template, name, params, expr)                 \
        _template                                                               \
        Vector<T, 2> name(params) {                                             \
            Vector<T, 2> result;                                                \
            { constexpr Usize i = 0; result[i] = (expr); }                      \
            { constexpr Usize i = 1; result[i] = (expr); }                      \
            return result;                                                      \
        }

    #define lpp_def_vector3_proc(_template, name, params, expr)                 \
        _template                                                               \
        Vector<T, 3> name(params) {                                             \
            Vector<T, 3> result;                                                \
            { constexpr Usize i = 0; result[i] = (expr); }                      \
            { constexpr Usize i = 1; result[i] = (expr); }                      \
            { constexpr Usize i = 2; result[i] = (expr); }                      \
            return result;                                                      \
        }

    #define lpp_def_vector4_proc(_template, name, params, expr)                 \
        _template                                                               \
        Vector<T, 4> name(params) {                                             \
            Vector<T, 4> result;                                                \
            { constexpr Usize i = 0; result[i] = (expr); }                      \
            { constexpr Usize i = 1; result[i] = (expr); }                      \
            { constexpr Usize i = 2; result[i] = (expr); }                      \
            { constexpr Usize i = 3; result[i] = (expr); }                      \
            return result;                                                      \
        }


    #define lpp_def_vector_proc_1(name, expr) \
        lpp_def_vector_proc(LPP_PASS(template <typename T, Usize n>), name, LPP_PASS(Vector<T, n> a), expr) \
        lpp_def_vector1_proc(LPP_PASS(template <typename T>), name, LPP_PASS(Vector<T, 1> a), expr) \
        lpp_def_vector2_proc(LPP_PASS(template <typename T>), name, LPP_PASS(Vector<T, 2> a), expr) \
        lpp_def_vector3_proc(LPP_PASS(template <typename T>), name, LPP_PASS(Vector<T, 3> a), expr) \
        lpp_def_vector4_proc(LPP_PASS(template <typename T>), name, LPP_PASS(Vector<T, 4> a), expr)

    #define lpp_def_vector_proc_1_simple(name) \
        lpp_def_vector_proc_1(name, LPP_PASS(name(a[i])))


    #define lpp_def_vector_proc_2(name, expr) \
        lpp_def_vector_proc(LPP_PASS(template <typename T, Usize n>), name, LPP_PASS(Vector<T, n> a, Vector<T, n> b), expr) \
        lpp_def_vector1_proc(LPP_PASS(template <typename T>), name, LPP_PASS(Vector<T, 1> a, Vector<T, 1> b), expr) \
        lpp_def_vector2_proc(LPP_PASS(template <typename T>), name, LPP_PASS(Vector<T, 2> a, Vector<T, 2> b), expr) \
        lpp_def_vector3_proc(LPP_PASS(template <typename T>), name, LPP_PASS(Vector<T, 3> a, Vector<T, 3> b), expr) \
        lpp_def_vector4_proc(LPP_PASS(template <typename T>), name, LPP_PASS(Vector<T, 4> a, Vector<T, 4> b), expr)

    #define lpp_def_vector_proc_2_simple(name) \
        lpp_def_vector_proc_2(name, LPP_PASS(name(a[i], b[i])))


    #define lpp_def_vector_scalar_proc(name, expr) \
        lpp_def_vector_proc(LPP_PASS(template <typename Scalar, typename T, Usize n>), name, LPP_PASS(Vector<T, n> vector, Scalar scalar), expr) \
        lpp_def_vector1_proc(LPP_PASS(template <typename Scalar, typename T>), name, LPP_PASS(Vector<T, 1> vector, Scalar scalar), expr) \
        lpp_def_vector2_proc(LPP_PASS(template <typename Scalar, typename T>), name, LPP_PASS(Vector<T, 2> vector, Scalar scalar), expr) \
        lpp_def_vector3_proc(LPP_PASS(template <typename Scalar, typename T>), name, LPP_PASS(Vector<T, 3> vector, Scalar scalar), expr) \
        lpp_def_vector4_proc(LPP_PASS(template <typename Scalar, typename T>), name, LPP_PASS(Vector<T, 4> vector, Scalar scalar), expr)


    #define lpp_def_scalar_vector_proc(name, expr) \
        lpp_def_vector_proc(LPP_PASS(template <typename Scalar, typename T, Usize n>), name, LPP_PASS(Scalar scalar, Vector<T, n> vector), expr) \
        lpp_def_vector1_proc(LPP_PASS(template <typename Scalar, typename T>), name, LPP_PASS(Scalar scalar, Vector<T, 1> vector), expr) \
        lpp_def_vector2_proc(LPP_PASS(template <typename Scalar, typename T>), name, LPP_PASS(Scalar scalar, Vector<T, 2> vector), expr) \
        lpp_def_vector3_proc(LPP_PASS(template <typename Scalar, typename T>), name, LPP_PASS(Scalar scalar, Vector<T, 3> vector), expr) \
        lpp_def_vector4_proc(LPP_PASS(template <typename Scalar, typename T>), name, LPP_PASS(Scalar scalar, Vector<T, 4> vector), expr)




    lpp_def_vector_proc_1(LPP_PASS(operator+), LPP_PASS(+a[i]))
    lpp_def_vector_proc_1(LPP_PASS(operator-), LPP_PASS(-a[i]))

    lpp_def_vector_proc_2(LPP_PASS(operator+), LPP_PASS(a[i] + b[i]))
    lpp_def_vector_proc_2(LPP_PASS(operator-), LPP_PASS(a[i] - b[i]))
    lpp_def_vector_proc_2(LPP_PASS(operator*), LPP_PASS(a[i] * b[i]))
    lpp_def_vector_proc_2(LPP_PASS(operator/), LPP_PASS(a[i] / b[i]))
    lpp_def_vector_proc_2(LPP_PASS(operator%), LPP_PASS(a[i] % b[i]))

    lpp_def_scalar_vector_proc(LPP_PASS(operator*), LPP_PASS(scalar*vector[i]))
    lpp_def_vector_scalar_proc(LPP_PASS(operator*), LPP_PASS(vector[i]*scalar))

    lpp_def_vector_scalar_proc(LPP_PASS(operator/), LPP_PASS(vector[i]/scalar))
    lpp_def_vector_scalar_proc(LPP_PASS(operator%), LPP_PASS(vector[i]%scalar))

}

