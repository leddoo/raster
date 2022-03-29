#include "common.hpp"
#include "simd/simd.hpp"

#include <cmath>


namespace lpp {

    F32 length(V2f v) {
        return std::sqrt(length_squared(v));
    }


    Void fill_copy_bytes(Addr begin, Addr end, Addr filled_until) {
        auto width = Usize(filled_until - begin);

        auto cursor = filled_until;
        while(cursor < end) {
            auto remaining = Usize(end - cursor);
            auto count = at_most(remaining, width);
            copy_bytes(cursor, begin, count);

            cursor += count;
            width *= 2;
        }
    }

}


namespace raster {


    inline U32 _pack(F32x4 color) {
        auto u8s = pack_with_unsigned_saturation(
            pack_with_signed_saturation(
                to_s32s(color)
            )
        );
        return reinterpret_cast<Ref<U32>>(u8s);
    }

    inline U32 _pack_255(F32x4 color) {
        return _pack(color * F32x4(255.0f));
    }


    inline F32x4 _unpack(U32 color) {
        auto u8s = interpret_as_u8s(U32x4(color));
        return to_f32s(unpack_low(unpack_low(u8s)));
    }

    inline F32x4 _unpack_255(U32 color) {
        return _unpack(color) / F32x4(255.0f);
    }


    Color_Rgba::Color_Rgba(U8 r, U8 g, U8 b, U8 a) {
        this->value = U32( (r << 0) | (g << 8) | (b << 16) | (a << 24) );
    }

    Color_Rgba Color_Rgba::pack_255(F32 r, F32 g, F32 b, F32 a) {
        return Color_Rgba(_pack_255(F32x4(r, g, b, a)));
    }

    Color_Rgba Color_Rgba::pack_255(V4f color) {
        return Color_Rgba(_pack_255(F32x4(color)));
    }

    Color_Rgba Color_Rgba::pack(F32 r, F32 g, F32 b, F32 a) {
        return Color_Rgba(_pack(F32x4(r, g, b, a)));
    }

    Color_Rgba Color_Rgba::pack(V4f color) {
        return Color_Rgba(_pack(F32x4(color)));
    }

    V4f Color_Rgba::unpack_255() const {
        return to_v4f(_unpack_255(this->value));
    }

    V4f Color_Rgba::unpack() const {
        return to_v4f(_unpack(this->value));
    }



    Color_Bgra::Color_Bgra(U8 r, U8 g, U8 b, U8 a) {
        this->value = U32( (b << 0) | (g << 8) | (r << 16) | (a << 24) );
    }

    Color_Bgra Color_Bgra::pack_255(F32 r, F32 g, F32 b, F32 a) {
        return Color_Bgra(_pack_255(F32x4(b, g, r, a)));
    }

    Color_Bgra Color_Bgra::pack_255(V4f color) {
        return Color_Bgra(_pack_255(shuffle_rgba_to_bgra(F32x4(color))));
    }

    Color_Bgra Color_Bgra::pack(F32 r, F32 g, F32 b, F32 a) {
        return Color_Bgra(_pack(F32x4(b, g, r, a)));
    }

    Color_Bgra Color_Bgra::pack(V4f color) {
        return Color_Bgra(_pack(shuffle_rgba_to_bgra(F32x4(color))));
    }

    V4f Color_Bgra::unpack_255() const {
        return to_v4f(shuffle_bgra_to_rgba(_unpack_255(this->value)));
    }

    V4f Color_Bgra::unpack() const {
        return to_v4f(shuffle_bgra_to_rgba(_unpack(this->value)));
    }


}

