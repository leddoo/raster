#pragma once

#include <lpp/core/basic.hpp>
#include <lpp/core/proto.hpp>

namespace lpp {

    namespace proto {

        /* Destroy protocol:
            - After destroy, an object is empty.
            - Implement for types that require clean up on destroy.
            - Needs to be called manually.
        */

        struct Destroy_Record {
            using destroy_Proc = Void(*)(Addr object);

            destroy_Proc destroy;
        };

        LPP_DEF_PROTO(Destroy, LPP_PASS(typename T), LPP_PASS(T));

    }


    LPP_API Void destroy      (Opt_Ptr<proto::Destroy_Record> record, Usize size, Addr object);
    // TODO: move to buffer.
    LPP_API Void destroy_array(Opt_Ptr<proto::Destroy_Record> record, Usize size, Addr base, Usize begin, Usize end);

    template <typename T>
    Void destroy(Ptr<T> object) {
        destroy(proto::Destroy<T>::get(), sizeof(T), Addr(object));
    }

    template <typename T>
    Void destroy_maybe(Ptr<T> object) {
        destroy(proto::Destroy<T>::get_maybe(), sizeof(T), Addr(object));
    }


    // TODO: move to basic type info.
    template <typename Type_Info>
    Void destroy(Ptr<const Type_Info> type_info, Addr object) {
        destroy(type_info->destroy, type_info->size, object);
    }

    // TODO: move to buffer.
    template <typename Type_Info>
    Void destroy_array(Ptr<const Type_Info> type_info, Addr base, Usize begin, Usize end) {
        destroy_array(type_info->destroy, type_info->size, base, begin, end);
    }
}

