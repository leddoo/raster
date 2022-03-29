#pragma once

#include <lpp/core/basic.hpp>
#include <lpp/core/move.hpp>
#include <lpp/core/destroy.hpp>

namespace lpp {

    struct Basic_Type_Info {
        Usize size;
        Usize alignment;
        Opt_Ptr<proto::Move_Record> move;
        Opt_Ptr<proto::Destroy_Record> destroy;

        template <typename T>
        static Ptr<Basic_Type_Info> get() {
            static Basic_Type_Info info = {
                sizeof(T), alignof(T),
                proto::Move<T>::get_maybe(),
                proto::Destroy<T>::get_maybe(),
            };
            return &info;
        }
    };

}

