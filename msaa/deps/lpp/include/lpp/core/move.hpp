#pragma once

#include <lpp/core/basic.hpp>
#include <lpp/core/proto.hpp>

namespace lpp {

    namespace proto {

        /* Move protocol:
            - Implement for types where copy_bytes is not enough to move a value
              to another memory location.
            - When implemented, move is destroy: meaning that the source object
              is empty after the move.
              Use LPP_MOVE_IS_DESTROY to implement move using copy_bytes (move
              is treated as destroy due to existence of implementation).
              Note: this is sub-optimal, as move_array cannot use a single
              copy_bytes call. This behavior may change in future versions of
              the library.
        */

        struct Move_Record {
            using move_Proc = Void(*)(Addr dst, Addr src);

            move_Proc move;
        };

        LPP_DEF_PROTO(Move, LPP_PASS(typename T), LPP_PASS(T));

    }

    #define LPP_MOVE_IS_DESTROY(params, arg)                                    \
        LPP_IMPL_PROTO(Move, LPP_PASS(params), LPP_PASS(arg), LPP_PASS(         \
            [](Addr dst, Addr src) {                                            \
                copy_bytes(dst, src, sizeof(arg));                              \
            },                                                                  \
        ))


    LPP_API Void move      (Opt_Ptr<proto::Move_Record> record, Usize size, Addr dst, Addr src);
    // TODO: move to buffer.
    LPP_API Void move_array(Opt_Ptr<proto::Move_Record> record, Usize size, Addr dst, Addr src, Usize length);

    template <typename T>
    Void move(Ptr<T> dst, Ptr<T> src) {
        // TODO: Remove_Pointer validation.
        move(proto::Move<T>::get_maybe(), sizeof(T), Addr(dst), Addr(src));
    }


    // TODO: move to basic type info.
    template <typename Type_Info>
    Void move(Ptr<const Type_Info> type_info, Addr dst, Addr src) {
        move(type_info->move, type_info->size, dst, src);
    }

    // TODO: move to buffer.
    template <typename Type_Info>
    Void move_array(Ptr<const Type_Info> type_info, Addr dst, Addr src, Usize length) {
        move_array(type_info->move, type_info->size, dst, src, length);
    }


    // TODO: dynamic variant?
    // TODO: consider ignoring aliasing. maybe return early if a == b.
    template <typename T>
    Void swap(Ptr<T> _a, Ptr<T> _b) {
        auto a = Addr(_a);
        auto b = Addr(_b);
        if(are_overlapping(a, b, sizeof(T))) {
            throw "Swap operands must not alias.";
        }

        alignas(alignof(T)) Byte buffer[sizeof(T)];
        auto t = &buffer[0];

        auto record = proto::Move<T>::get_maybe();
        if(record.is_some()) {
            record.value->move(t, a);
            record.value->move(a, b);
            record.value->move(b, t);
        }
        else {
            copy_bytes(t, a, sizeof(T));
            copy_bytes(a, b, sizeof(T));
            copy_bytes(b, t, sizeof(T));
        }
    }

}

