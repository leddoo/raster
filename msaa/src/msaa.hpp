#pragma once

#include "common.hpp"
#include "rasterizer.hpp"


namespace raster {
namespace msaa {

    enum class Samples : U16 {
        x2, x4, x8, x16, x32,
    };


    struct Lut {
        static constexpr U16 max_sample_count   = 32;
        static constexpr U16 default_resolution = 128;
        static constexpr F32 default_range      = 0.7071067811865475244f; // sqrt(2)/2


        static Lut create(
            Samples samples,
            U16 resolution = default_resolution,
            F32 range = default_range
        );

        static Lut create(
            Ptr<const V2f> samples, U16 sample_count,
            U16 resolution = default_resolution,
            F32 range = default_range
        );


        U32 fetch(V2f n, F32 a) const;
        U32 fetch_point_01(V2f n, V2f point) const;
        U32 fetch_y_left(V2f n, F32 y_left) const;


        List<U32> table;
        Ptr<const V2f> samples;
        U16 resolution;
        U16 sample_count;
        F32 range;

        F32 resolution_f32;
        F32 inv_range;
        F32 min_a;
        U32 sample_mask;

        Lut() {}
        LPP_MOVE_IS_DESTROY_CTORS(Lut, Lut);
    };


    struct Sample_Run {
        V2s position;
        U32 length;
        U32 sample_mask;
    };

    Void rasterize(
        Ref<const List<Segment<V2f>>> segments,
        Ref<const Lut> lut,
        Ref<List<Sample_Run>> sample_runs
    );


    struct Rasterizer : raster::Rasterizer {
        Ptr<const Lut> lut;
        Ptr<List<Sample_Run>> sample_runs;

        List<V2f> normals;
        U8        scan_winding;

        Rasterizer(Ptr<const Lut> lut, Ptr<List<Sample_Run>> sample_runs) : lut(lut), sample_runs(sample_runs) {}

        virtual Void on_init() override;
        virtual Void on_scanline() override;
        virtual Void on_fragment() override;

        virtual Void _destroy() override;

        Void add_sample_run(V2s position, U32 length, U32 sample_mask);

        LPP_MOVE_IS_DESTROY_CTORS(Rasterizer, Rasterizer);
    };


    Void resolve(
        Ref<Image<Color_Bgra>> dst,
        Ref<const Image<Color_Rgba>> src,
        Bool un_pre_multiply_alpha = false
    );

    Void fill_opaque(
        Ref<Image<Color_Rgba>> image,
        Ref<const List<Sample_Run>> sample_runs,
        V4f color
    );

}}

