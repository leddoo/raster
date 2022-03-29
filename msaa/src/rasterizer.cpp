#include "rasterizer.hpp"

#include <algorithm>


namespace raster {

    Void Rasterizer::run(Ref<const List<Segment<V2f>>> segments) {
        this->init(segments);

        while(this->advance_scanline()) {
            while(this->advance_fragment()) {
            }
        }
    }


    inline V2f get_intersection(U8 axis, F32 limit, F32 target, V2f p0, V2f p1) {
        if(limit <= target) {
            return p1;
        }
        else {
            auto t = inverse_lerp(target, p0[axis], p1[axis]);
            t = clamp(t, 0.0f, 1.0f);

            auto value = lerp(p0[1u - axis], p1[1u - axis], t);

            V2f result;
            if(axis == 0) {
                result.x() = target;
                result.y() = value;
            }
            else {
                result.x() = value;
                result.y() = target;
            }
            return result;
        }
    }

    Void Rasterizer::init(Ref<const List<Segment<V2f>>> segments) {
        // create infos.
        this->infos.reserve(segments.length);
        this->infos.length = 0;
        for(auto segment : segments) {
            this->infos.append_new(segment);
        }

        // sort infos by y.
        std::sort(
            this->infos.begin().value, this->infos.end().value,
            [](const auto& a, const auto& b) {
                return a.get_y_min() <= b.get_y_min();
            }
        );


        // init scanline.
        auto scan = &this->scanline;
        {
            scan->segments.set_length(this->infos.length);
            scan->actives.length = 0;
            scan->info_cursor    = 0;
            scan->position       = s32_max;
            scan->next_position  = s32_max;

            if(this->infos.length > 0) {
                scan->next_position = floor_to_s32(this->infos[0].get_y_min());
            }
        }

        // init fragments.
        auto frag = &this->fragment;
        {
            frag->segments.set_length(this->infos.length);
        }

        this->on_init();
    }

    Bool Rasterizer::advance_scanline() {
        auto scan = &this->scanline;
        auto frag = &this->fragment;

        // stop condition.
        if(scan->actives.length == 0 && scan->info_cursor >= this->infos.length) {
            return false;
        }

        // advance position.
        {
            scan->position      = scan->next_position;
            scan->next_position = scan->position + 1;
        }


        // add newly active segments.
        while( scan->info_cursor < this->infos.length
            && this->infos[scan->info_cursor].get_y_min() < this->scanline_end()
        ) {
            auto segment_index = scan->info_cursor;
            auto& info = this->infos[segment_index];

            this->get_scan_segment_top_point(segment_index) = info.get_bottom_point();

            scan->actives.append_new(scan->info_cursor);
            scan->info_cursor += 1;
        }


        constexpr auto removed_marker = U32(-1);

        // update actives.
        for(auto& segment_index : scan->actives) {
            const auto& info = this->infos[segment_index];
            auto& segment    = scan->segments[segment_index];

            if(info.get_y_max() <= this->scanline_begin()) {
                segment_index = removed_marker;
            }
            else {
                // we walk the segment bottom to top.
                auto& bottom = this->get_scan_segment_bottom_point(segment_index);
                auto& top    = this->get_scan_segment_top_point(segment_index);
                bottom = top;
                top = get_intersection(
                    1, info.get_y_max(), F32(this->scanline_end()),
                    info.get_bottom_point(), info.get_top_point()
                );


                // y_mid intersection.
                auto y_mid = F32(scan->position) + 0.5f;

                // compute flags.
                segment.left_leq_y_mid  = (segment.left().y()  <= y_mid);
                segment.right_leq_y_mid = (segment.right().y() <= y_mid);

                // compute intersection.
                auto y_min = bottom.y();
                auto y_max = top.y();
                if(y_min <= y_mid && y_max > y_mid) {
                    auto dy = y_max - y_min;
                    auto t = 0.5f;
                    if(dy > 5e-6f) {
                        // absolute error should be fine.
                        // don't think we need to clamp t.
                        t = (y_mid - y_min) / dy;
                    }

                    auto position = lerp(bottom.x(), top.x(), t);
                    auto fragment = lpp::floor(position);
                    segment.y_mid_fragment = S32(fragment);
                }
                else {
                    segment.y_mid_fragment = s32_max;
                }
            }
        }


        // sort active by x_min.
        bubble_sort_right_to_left(scan->actives, [&](U32 a_index, U32 b_index) -> Bool {
            auto a = FLT_MAX;
            if(a_index != removed_marker) {
                a = scan->segments[a_index].left().x();
            }

            auto b = FLT_MAX;
            if(b_index != removed_marker) {
                b = scan->segments[b_index].left().x();
            }

            return a <= b;
        });

        // remove trailing removed_markers.
        while( scan->actives.length > 0
            && scan->actives.last_unchecked() == removed_marker
        ) {
            scan->actives.length -= 1;
        }


        // init fragment.
        {
            frag->actives.length         = 0;
            frag->scanline_active_cursor = 0;
            frag->position               = s32_max;
            frag->next_position          = s32_max;
            frag->next_segment_position  = s32_max;

            auto x_min = F32();
            if(this->get_next_scanline_active_x_min(x_min)) {
                frag->next_position         = floor_to_s32(x_min);
                frag->next_segment_position = frag->next_position;
            }
        }

        this->on_scanline();

        return true;
    }

    Bool Rasterizer::advance_fragment() {
        auto scan = &this->scanline;
        auto frag = &this->fragment;

        // stop condition.
        if(frag->actives.length == 0 && frag->scanline_active_cursor >= scan->actives.length) {
            return false;
        }

        // advance position.
        {
            frag->position      = frag->next_position;
            frag->next_position = frag->position + 1;
        }


        // add newly live segments.
        if(frag->position >= frag->next_segment_position) {
            auto x_min = F32();
            while( this->get_next_scanline_active_x_min(x_min)
                && x_min <= this->fragment_end()
            ) {
                auto segment_index = scan->actives[frag->scanline_active_cursor];

                auto& segment = frag->segments[segment_index];
                segment.right() = scan->segments[segment_index].left();

                frag->actives.append_new(segment_index);
                frag->scanline_active_cursor += 1;
            }

            // update next_segment_position.
            if(this->get_next_scanline_active_x_min(x_min)) {
                frag->next_segment_position = floor_to_s32(x_min);
            }
            else {
                frag->next_segment_position = s32_max;
            }
        }

        // update actives.
        for(auto i = Usize(0); i < frag->actives.length; /* nop */) {
            auto segment_index = frag->actives[i];

            auto& scan_segment = scan->segments[segment_index];
            auto& frag_segment = frag->segments[segment_index];

            auto left  = scan_segment.left();
            auto right = scan_segment.right();

            if(right.x() <= this->fragment_begin()) {
                // TODO: remove swap.
                frag->actives[i] = frag->actives.last_unchecked();
                frag->actives.length -= 1;
            }
            else {
                frag_segment.left() = frag_segment.right();
                frag_segment.right() = get_intersection(
                    0, right.x(), F32(this->fragment_end()),
                    left, right
                );

                i += 1;
            }
        }

        // skip spans.
        if(frag->actives.length == 0) {
            frag->next_position = frag->next_segment_position;
        }

        this->on_fragment();

        return true;
    }

    Bool Rasterizer::get_next_scanline_active_x_min(Ref<F32> x_min) {
        auto scan = &this->scanline;
        auto frag = &this->fragment;

        if(frag->scanline_active_cursor < scan->actives.length) {
            auto segment_index = scan->actives[frag->scanline_active_cursor];
            x_min = scan->segments[segment_index].left().x();

            return true;
        }
        else {
            return false;
        }
    }

    Ref<V2f> Rasterizer::get_scan_segment_bottom_point(U32 index) {
        if(this->infos[index].bottom_is_left()) {
            return this->scanline.segments[index].left();
        }
        else {
            return this->scanline.segments[index].right();
        }
    }

    Ref<V2f> Rasterizer::get_scan_segment_top_point(U32 index) {
        if(this->infos[index].bottom_is_left()) {
            return this->scanline.segments[index].right();
        }
        else {
            return this->scanline.segments[index].left();
        }
    }

    Void Rasterizer::_destroy() {
        this->infos._destroy();
        this->scanline.segments._destroy();
        this->scanline.actives._destroy();
        this->fragment.segments._destroy();
        this->fragment.actives._destroy();
    }

    Rasterizer::Segment_Info::Segment_Info(Segment<V2f> segment) {
        auto x0 = segment.p0().x();
        auto x1 = segment.p1().x();
        auto y0 = segment.p0().y();
        auto y1 = segment.p1().y();

        this->segment          = segment;
        this->winding          = 0;
        this->left_point_index = 0;
        this->is_horizontal    = (y0 == y1);
        this->is_vertical      = (x0 == x1);

        if(this->is_horizontal) {
            if(x0 < x1) {
                // right
                this->winding = U8(-1);
                this->left_point_index = 0;
            }
            else if(x0 > x1) {
                // left: convert to right.
                std::swap(this->segment.p0(), this->segment.p1());
                this->winding = U8(+1);
                this->left_point_index = 0;
            }
        }
        else {
            if(y0 < y1) {
                // up
                this->winding = U8(+1);
                this->left_point_index = U8((x0 <= x1) ? 0 : 1);
            }
            else if(y0 > y1) {
                // down: convert to up.
                std::swap(this->segment.p0(), this->segment.p1());
                this->winding = U8(-1);
                this->left_point_index = U8((x1 <= x0) ? 0 : 1);
            }
        }
    }

}

