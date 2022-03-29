#pragma once

#include <lpp/core/basic.hpp>
#include <lpp/memory/allocator.hpp>
#include <lpp/memory/list.hpp>

namespace lpp {

    /* Arena
        - Monotonic memory allocator.
        - Use save and restore to free memory.
          The LPP_ARENA_TEMP_SCOPE macro does this for you: it saves the arena
          state at the invocation and restores it at the end of the current block.
    */
    struct LPP_API Arena {
        static constexpr Usize alignment_padding  = 2*sizeof(Addr);
        static constexpr Usize default_block_size = 512*1024 - alignment_padding;

        struct Block {
            Addr begin;
            Addr end;
            Addr used_end;

            Usize remaining() const;
            Usize size() const;
        };

        struct Marker {
            Usize     blocks_length;
            Addr used_end;
        };


        Usize            block_size;
        Ptr<Allocator>   block_allocator;
        Ptr<List<Block>> blocks;



        Arena(
            Ptr<Allocator> block_allocator LPP_USE_DEFAULT_ALLOCATOR,
            Ptr<Allocator> list_allocator LPP_USE_DEFAULT_ALLOCATOR,
            Usize block_size = default_block_size
        );

        LPP_MOVE_IS_DESTROY_CTORS(Arena, Arena);


        Void _destroy();


        Addr allocate(Usize size, Usize alignment);

        Marker save();
        Void   restore(Marker marker);


        Opt_Addr _get_aligned_pointer(Usize size, Usize alignment);

        Void _push_block(Usize size);
    };

    #define LPP_ARENA_TEMP_SCOPE(arena)                                         \
        auto LPP_CONCAT(__lpp_arena_marker, __LINE__) = (arena).save();         \
        lpp_defer { (arena).restore(LPP_CONCAT(__lpp_arena_marker, __LINE__)); }


    namespace proto {
        LPP_MOVE_IS_DESTROY(LPP_PASS(), LPP_PASS(Arena));

        LPP_IMPL_PROTO(Destroy, LPP_PASS(), LPP_PASS(Arena), LPP_PASS(
            [](Addr object) {
                Ptr<Arena>(object)->_destroy();
            },
        ));

        LPP_IMPL_PROTO(Allocator, LPP_PASS(), LPP_PASS(Arena), LPP_PASS(
            [](Addr self, Usize size, Usize alignment) -> Addr {
                return Ptr<Arena>(self)->allocate(size, alignment);
            },
            nullptr,
        ));
    }

}

