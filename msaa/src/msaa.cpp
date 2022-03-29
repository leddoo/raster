#include "msaa.hpp"

#include "rasterizer.hpp"
#include "simd/simd.hpp"

#include <cstdio>
#include <cassert>


namespace raster {
namespace msaa {

    const V2f samples_x2[2] = {
        V2f({-4, -4}), V2f({ 4,  4}),
    };

    const V2f samples_x4[4] = {
        V2f({-2, -6}), V2f({ 7, -2}), V2f({-6,  2}), V2f({ 2,  6}),
    };

    const V2f samples_x8[8] = {
        V2f({ 7, -7}), V2f({-3, -5}), V2f({ 1, -3}), V2f({-7, -1}),
        V2f({ 5,  1}), V2f({-1,  3}), V2f({-5,  5}), V2f({ 3,  7}),
    };

    const V2f samples_x16[16] = {
        V2f({-7, -8}), V2f({ 0, -7}), V2f({-4, -6}), V2f({ 3, -5}),
        V2f({ 7, -4}), V2f({-1, -3}), V2f({-5, -2}), V2f({ 4, -1}),
        V2f({-8,  0}), V2f({ 1,  1}), V2f({-3,  2}), V2f({ 5,  3}),
        V2f({-6,  4}), V2f({ 2,  5}), V2f({-2,  6}), V2f({ 6,  7}),
    };

    const V2f samples_x32[32] = {
        V2f({-4, -7}), V2f({ 5, -7}), V2f({ 1, -6}), V2f({-7, -5}),
        V2f({-3, -5}), V2f({ 6, -5}), V2f({ 5, -4}), V2f({-1, -4}),
        V2f({ 4, -4}), V2f({ 2, -3}), V2f({-2, -2}), V2f({ 7, -2}),
        V2f({-6, -1}), V2f({ 1, -1}), V2f({ 3, -1}), V2f({-4,  0}),
        V2f({-7,  1}), V2f({ 2,  1}), V2f({-1,  2}), V2f({ 6,  2}),
        V2f({-6,  3}), V2f({-3,  3}), V2f({ 0,  4}), V2f({ 4,  4}),
        V2f({ 2,  5}), V2f({ 7,  5}), V2f({-7,  6}), V2f({-3,  6}),
        V2f({ 5,  6}), V2f({-5,  7}), V2f({-1,  7}), V2f({ 3,  7}),
    };


    Lut Lut::create(Samples samples, U16 resolution, F32 range) {
        switch(samples) {
            case Samples::x2:  { return Lut::create(samples_x2,   2, resolution, range); } break;
            case Samples::x4:  { return Lut::create(samples_x4,   4, resolution, range); } break;
            case Samples::x8:  { return Lut::create(samples_x8,   8, resolution, range); } break;
            case Samples::x16: { return Lut::create(samples_x16, 16, resolution, range); } break;
            case Samples::x32: { return Lut::create(samples_x32, 32, resolution, range); } break;
            default: throw "Unreachable.";
        }
    }

    Lut Lut::create(
        Ptr<const V2f> samples, U16 sample_count,
        U16 resolution, F32 range
    ) {
        if(sample_count > max_sample_count) {
            throw "Invalid sample count.";
        }

        auto lut = Lut();
        lut.table.set_length(Usize(resolution*resolution));
        lut.samples      = samples;
        lut.resolution   = resolution;
        lut.sample_count = sample_count;
        lut.range        = range;

        lut.resolution_f32 = F32(resolution);
        lut.inv_range = 1.0f/range;
        lut.min_a     = 1.0f/F32(resolution) * range;

        lut.sample_mask = mask_ending_at<U32>(sample_count);

        for(auto y : Range<Usize>(resolution)) {
            for(auto x : Range<Usize>(resolution)) {
                auto tex_coord = (V2f({F32(x), F32(y)}) + V2f(0.5f)) / F32(resolution);
                auto p = 2.0f*(tex_coord - V2f(0.5f));
                auto n = normalized(p);
                auto a = (1.0f - dot(n, p))*range;

                auto mask = U32(0);
                for(auto i : Range<Usize>(sample_count)) {
                    auto sample = samples[i] / 16.0f;
                    mask |= (dot(n, sample) > a) << i;
                }
                lut.table[y*resolution + x] = mask;
            }
        }

        return lut;
    }


    U32 Lut::fetch(V2f n, F32 a) const {
        auto flip = false;
        if(a < 0.0f) {
            a = -a;
            n = -n;
            flip = true;
        }
        a = clamp(a, this->min_a, this->range);

        auto p = (1.0f - a*this->inv_range)*n;
        auto tex_coord = 0.5f*p + V2f(0.5f);

        auto x = U32(tex_coord.x()*this->resolution_f32);
        auto y = U32(tex_coord.y()*this->resolution_f32);

        #if LPP_DEBUG
            assert(x < this->resolution);
            assert(y < this->resolution);
        #endif

        auto mask = this->table[y*this->resolution + x];
        if(flip) {
            return ~mask;
        }
        else {
            return mask;
        }
    }

    U32 Lut::fetch_point_01(V2f n, V2f point) const {
        auto r = point;
        r.x() -= 0.5f;
        r.y() -= 0.5f;
        return this->fetch(n, dot(n, r));
    }

    U32 Lut::fetch_y_left(V2f n, F32 y_left) const {
        return this->fetch_point_01(n, V2f({ 0.0f, y_left }));
    }

    Void print(Ptr<const Lut> lut, U32 mask, V2f offset = V2f()) {
        for(auto i : Range<Usize>(lut->sample_count)) {
            if((mask & (1 << i)) != 0) {
                auto p = lut->samples[i]/16.0f + V2f(0.5f) + offset;
                printf("Point({ %f, %f }), ", p.x(), p.y());
            }
        }
    }



    Void rasterize(
        Ref<const List<Segment<V2f>>> segments,
        Ref<const Lut> lut,
        Ref<List<Sample_Run>> sample_runs
    ) {
        auto rasterizer = Rasterizer(&lut, &sample_runs);
        rasterizer.run(segments);
        rasterizer._destroy();
    }


    void Rasterizer::on_init() {
        this->normals.reserve(this->infos.length);
        this->normals.length = 0;
        this->scan_winding = 0;

        for(const auto& info : this->infos) {
            auto delta = info.get_top_point() - info.get_bottom_point();
            auto normal = normalized(rotated_cw(delta));
            this->normals.append_new(normal);
        }
    }

    void Rasterizer::on_scanline() {
        #if LPP_DEBUG
        assert(this->scan_winding == 0);
        #endif
        this->scan_winding = 0;
    }

    void Rasterizer::on_fragment() {
        auto scan = &this->scanline;
        auto frag = &this->fragment;

        auto x_begin = F32(this->fragment_begin());
        auto y_begin = F32(this->scanline_begin());
        auto y_end   = F32(this->scanline_end());

        auto frag_pos = V2f({ x_begin, y_begin });

        auto frag_pos_s32 = V2s({ this->fragment_begin(), this->scanline_begin() });

        if(frag->actives.length > 0) {
            // fragment.

            auto add_winding = [](Ref<Array<U8, 32>> windings, U8 winding) -> Void {
                auto winding_x16 = U8x16(winding);

                auto cursor = Ptr<U8x16>(windings.begin());
                cursor->store(cursor->load() + winding_x16);
                cursor += 1;
                cursor->store(cursor->load() + winding_x16);
            };

            auto add_winding_masked = [](Ref<Array<U8, 32>> windings, U32 mask, U8 winding) -> Void {
                auto winding_x16 = U8x16(winding);

                auto cursor = Ptr<U8x16>(windings.begin());
                cursor->store(masked_add(cursor->load(), winding_x16, U16(mask)));
                cursor += 1;
                cursor->store(masked_add(cursor->load(), winding_x16, U16(mask >> 16)));
            };

            auto to_mask_non_zero = [](Ref<Array<U8, 32>> windings) -> U32 {
                auto zero_x16 = U8x16(0);

                auto cursor = Ptr<U8x16>(windings.begin());

                auto zero_mask = U32(0);
                zero_mask |= (cursor->load() == zero_x16).high_bits_to_mask();
                cursor += 1;
                zero_mask |= (cursor->load() == zero_x16).high_bits_to_mask() << 16;

                auto non_zero_mask = ~zero_mask;
                return non_zero_mask;
            };


            // accumulate winding deltas.
            auto scan_delta = U8(0);
            auto sample_deltas = Array<U8, 32>();
            for(auto segment_index : frag->actives) {
                const auto& info     = this->infos[segment_index];
                const auto& scan_seg = scan->segments[segment_index];
                const auto& frag_seg = frag->segments[segment_index];
                auto left  = frag_seg.left();
                auto right = frag_seg.right();

                // skip zero length segments.
                if(left.x() == right.x() && left.y() == right.y()) {
                    continue;
                }

                auto intersects_main_ray = (x_begin == scan_seg.y_mid_fragment);

                // vertical segments touching the left edge are treated as
                // if they were in the previous fragment.
                if(left.x() == x_begin && info.is_vertical) {
                    if(intersects_main_ray) {
                        this->scan_winding += info.winding;
                    }

                    continue;
                }

                // scan winding.
                if(intersects_main_ray) {
                    scan_delta += info.winding;
                }

                // sample masks.
                auto low_mask = U32(-1);
                auto high_mask = U32(0);
                auto normal_mask = U32(0);
                {
                    auto y_min = min(left.y(), right.y());
                    auto y_max = max(left.y(), right.y());

                    /// TODO: cache.
                    if(y_min > y_begin) {
                        low_mask = this->lut->fetch_y_left(V2f({ 0.0f, 1.0f }), y_min - y_begin);
                    }

                    if(y_max < y_end) {
                        high_mask = this->lut->fetch_y_left(V2f({ 0.0f, 1.0f }), y_max - y_begin);
                    }

                    normal_mask = this->lut->fetch_point_01(normals[segment_index], left - frag_pos);
                }

                // horizontal ray.
                {
                    auto horizontal_mask = low_mask & (~high_mask) & normal_mask;
                    add_winding_masked(sample_deltas, horizontal_mask, info.winding);
                }

                // vertical ray.
                if(left.x() == x_begin) {
                    auto vertical_winding = info.winding;
                    auto vertical_mask = info.bottom_is_left() ? low_mask : high_mask;

                    // edge cases. refer to notes.
                    /// TODO: provide notes.
                    auto is_up = info.bottom_is_left() && _not(info.is_horizontal);
                    if(is_up) {
                        vertical_winding = U8(-vertical_winding);
                    }

                    // this is to deal with (almost) horizontal segments.
                    // the problem is that lerp(a, a, t) doesn't always return a.
                    auto left_leq_y_mid = scan_seg.left_leq_y_mid;
                    if(x_begin > scan_seg.y_mid_fragment) {
                        left_leq_y_mid = scan_seg.right_leq_y_mid;
                    }

                    if(left_leq_y_mid) {
                        vertical_winding = U8(-vertical_winding);
                        vertical_mask = ~vertical_mask;
                    }

                    add_winding_masked(sample_deltas, vertical_mask, vertical_winding);
                }
            }

            add_winding(sample_deltas, this->scan_winding);
            auto fragment_mask = to_mask_non_zero(sample_deltas);
            this->add_sample_run(frag_pos_s32, 1, fragment_mask);

            this->scan_winding += scan_delta;
        }
        else if(frag->scanline_active_cursor < scan->actives.length) {
            // span.
            if(this->scan_winding != 0) {
                auto length = U32(frag->next_segment_position - this->fragment_begin());
                this->add_sample_run(frag_pos_s32, length, U32(-1));
            }
        }
    }

    Void Rasterizer::_destroy() {
        raster::Rasterizer::_destroy();
        this->normals._destroy();
    }

    Void Rasterizer::add_sample_run(V2s position, U32 length, U32 sample_mask) {
        sample_mask &= this->lut->sample_mask;

        // NOTE: fill does not implement this yet.
        #if 0
        if(this->sample_runs->length > 0) {
            auto& last = this->sample_runs->last_unchecked();
            if(    sample_mask  == last.sample_mask
                && position.y() == last.position.y()
                && position.x() == last.position.x() + last.position.y()
            ) {
                last.length += length;
                return;
            }
        }
        #endif

        this->sample_runs->append_new(Sample_Run{ position, length, sample_mask });
    }


    Void resolve(
        Ref<Image<Color_Bgra>> dst,
        Ref<const Image<Color_Rgba>> src,
        Bool un_pre_multiply_alpha
    ) {
        assert(dst.lengths.x() == src.lengths.x());
        assert(dst.lengths.y() == src.lengths.y());
        assert(dst.sample_count == 1);

        auto dst_cursor = &dst.samples[0];
        auto src_cursor = &src.samples[0];

        auto scale_factor = V4f(1.0f/F32(src.sample_count));

        for(auto sample : Range<U32>(dst.lengths.x() * dst.lengths.y())) { LPP_UNUSED(sample);
            auto end = src_cursor + src.sample_count;

            auto color_0 = V4f();
            auto color_1 = V4f();
            auto color_2 = V4f();
            auto color_3 = V4f();
            while(src_cursor + 4 <= end) {
                color_0 = color_0 + (src_cursor + 0)->unpack();
                color_1 = color_1 + (src_cursor + 1)->unpack();
                color_2 = color_2 + (src_cursor + 2)->unpack();
                color_3 = color_3 + (src_cursor + 3)->unpack();
                src_cursor += 4;
            }

            auto color = (color_0 + color_1) + (color_2 + color_3);
            while(src_cursor < end) {
                color = color + src_cursor->unpack();
                src_cursor += 1;
            }

            color = color * scale_factor;

            if(un_pre_multiply_alpha) {
                auto a = color.a();
                if(a > 0.0f) {
                    color = to_v4f(F32x4(color) * (F32x4(255.0f) / F32x4(a)));
                    color.a() = a;
                }
            }

            *dst_cursor = Color_Bgra::pack(color);
            dst_cursor += 1;
        }
    }


    Void fill_opaque(
        Ref<Image<Color_Rgba>> image,
        Ref<const List<Sample_Run>> sample_runs,
        V4f color
    ) {
        auto packed    = Color_Rgba::pack_255(color);
        auto packed_x4 = U32x4(packed.value);

        auto all_samples = mask_ending_at<U32>(image.sample_count);

        for(const auto& run : sample_runs) {
            auto x_begin = U32(clamp(run.position.x(),                   0, S32(image.lengths.x())));
            auto x_end   = U32(clamp(run.position.x() + S32(run.length), 0, S32(image.lengths.x())));
            auto length = x_end - x_begin;

            if(    length == 0
                || run.position.y() < 0
                || U32(run.position.y()) >= image.lengths.y()
            ) {
                continue;
            }

            auto begin = image.get_first_sample(x_begin, U32(run.position.y()));
            auto end   = begin + length*image.sample_count;

            if(run.sample_mask == all_samples) {
                auto cursor = begin;

                // fill first pixel.
                auto first_end = begin + image.sample_count;
                while(cursor + 4 <= first_end) {
                    Ptr<U32x4>(cursor)->store(packed_x4);
                    cursor += 4;
                }
                while(cursor < first_end) {
                    *cursor = packed;
                    cursor += 1;
                }

                // copy first pixel to remainder of span.
                fill_copy_bytes(Addr(begin), Addr(end), Addr(cursor));
            }
            else if(length == 1) {
                // fill pixel creating masks ad hoc.

                auto cursor = begin;
                auto sample_mask = run.sample_mask;

                while(cursor + 4 <= end) {
                    auto mask_x4 = U32x4::unpack_bits(sample_mask);

                    auto at = Ptr<U32x4>(cursor);
                    at->store(
                          (at->load() & ~mask_x4)
                        | (packed_x4  & mask_x4)
                    );

                    cursor += 4;
                    sample_mask >>= 4;
                }
                while(cursor < end) {
                    auto mask = mask_all_equal<U32>(sample_mask & 0x1);

                    auto at = cursor;
                    *at = Color_Rgba(
                          (at->value    & ~mask)
                        | (packed.value & mask)
                    );

                    cursor += 1;
                    sample_mask >>= 1;
                }
            }
            else {
                throw "Unimplemented.";
                // cache masks.
                // loop using cached masks.
            }
        }

    }

}}

