#pragma once

#include <lpp/core/basic.hpp>
#include <lpp/core/proto.hpp>

namespace lpp {

    namespace proto {

        /* equal protocol.
        */

        struct Equal_Record {
            using equal_Proc = Bool(*)(Addr a, Addr b);

            equal_Proc equal;
        };

        LPP_DEF_PROTO(Equal, LPP_PASS(typename T), LPP_PASS(T));
    }


    template <typename T>
    Bool equal(Ref<const T> a, Ref<const T> b) {
        return proto::Equal<T>::get()->equal(Addr(&a), Addr(&b));
    }
}

