#pragma once

#include <lpp/core/basic.hpp>
#include <lpp/core/proto.hpp>

namespace lpp {

    namespace proto {

        /* hash protocol.
            - TODO: improve this.
        */
        struct Hash_Record {
            using hash_Proc = Void(*)(Addr object, Addr hash_value);

            hash_Proc hash;
        };

        LPP_DEF_PROTO(Hash, LPP_PASS(typename T, typename Result), LPP_PASS(T, Result));
    }


    template <typename Result, typename T>
    Result hash(Ref<const T> object) {
        auto result = Result();
        proto::Hash<T, Result>::get()->hash(Addr(&object), Addr(&result));
        return result;
    }
}

