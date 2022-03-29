#pragma once

#include "common.hpp"


namespace raster {


    struct Rasterizer {
        Void run(Ref<const List<Segment<V2f>>> segments);

        Void init(Ref<const List<Segment<V2f>>> segments);
        Bool advance_scanline();
        Bool advance_fragment();

        virtual Void on_init()     {}
        virtual Void on_scanline() {}
        virtual Void on_fragment() {}

        virtual Void _destroy();



        struct Segment_Info {
            Segment<V2f> segment;
            U8   winding;
            U8   left_point_index;
            Bool is_horizontal;
            Bool is_vertical;

            Segment_Info(Segment<V2f> segment);


            V2f get_bottom_point() const { return this->segment[0];                           }
            V2f get_top_point()    const { return this->segment[1];                           }
            V2f get_left_point()   const { return this->segment[this->left_point_index];      }
            V2f get_right_point()  const { return this->segment[1u - this->left_point_index]; }

            Bool bottom_is_left() const { return this->left_point_index == 0; }

            F32 get_y_min() const { return this->get_bottom_point().y(); }
            F32 get_y_max() const { return this->get_top_point().y();    }
        };


        struct Scan_Segment {
            Segment<V2f> segment;

            // y_mid intersection.
            S32  y_mid_fragment;
            Bool left_leq_y_mid;
            Bool right_leq_y_mid;

            Ref<V2f> left()  { return this->segment.p0(); }
            Ref<V2f> right() { return this->segment.p1(); }
        };


        struct Frag_Segment {
            Segment<V2f> segment;

            Ref<V2f> left()  { return this->segment.p0(); }
            Ref<V2f> right() { return this->segment.p1(); }
            V2f left()  const { return this->segment.p0(); }
            V2f right() const { return this->segment.p1(); }
        };



        List<Segment_Info> infos;

        struct {
            List<Scan_Segment> segments;
            List<U32> actives;
            U32 info_cursor;
            S32 position;
            S32 next_position;
        } scanline;

        struct {
            List<Frag_Segment> segments;
            List<U32> actives;
            U32 scanline_active_cursor;
            S32 position;
            S32 next_position;
            S32 next_segment_position;
        } fragment;


        Bool get_next_scanline_active_x_min(Ref<F32> x_min);

        Ref<V2f> get_scan_segment_bottom_point(U32 index);
        Ref<V2f> get_scan_segment_top_point(U32 index);

        S32 scanline_begin() const { return this->scanline.position; }
        S32 scanline_end()   const { return this->scanline.position + 1; }

        S32 fragment_begin() const { return this->fragment.position; }
        S32 fragment_end()   const { return this->fragment.position + 1; }

        Rasterizer() {}
        LPP_MOVE_IS_DESTROY_CTORS(Rasterizer, Rasterizer);
    };

}

