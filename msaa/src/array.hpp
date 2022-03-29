#pragma once

#include <lpp/core/basic.hpp>
#include "array_like.hpp"

namespace lpp {

    template <typename T, Usize n>
    struct Array {
        T _values[n] = {};

        Array() {}

        Array(const T (&values)[n]) {
            for(auto i : this->range()) {
                this->values()[i] = values[i];
            }
        }

        template <typename U>
        explicit Array(Ref<const Array<U, n>> other) {
            for(auto i : this->range()) {
                this->values()[i] = T(other.values()[i]);
            }
        }


        static constexpr Usize length() { return n; }

        Ptr<T>       values()       { return &this->_values[0]; }
        Ptr<const T> values() const { return &this->_values[0]; }

        lpp_array_like_def_accessors
    };

}

