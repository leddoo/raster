#pragma once

#include <lpp/core/basic.hpp>

namespace lpp {

    // TODO: these rely on range based for to be expanded as "it != end". it
    // would probably be more robust to actually implement != and create the
    // iterators with "clamping".

    // "monotonic" because of less than comparison.
    template <typename T>
    struct Monotonic_Iterator {
        T value;

        Monotonic_Iterator(T value) : value(value) {}

        T operator*() const { return this->value; }

        void operator++() { this->value += 1; }

        Bool operator!=(const Monotonic_Iterator<T>& other) const {
            return this->value < other.value;
        }
    };


    // also monotonic.
    template <typename T>
    struct Ptr_Iterator {
        Ptr<T> value;

        Ptr_Iterator(Ptr<T> value) : value(value) {}

        Ref<T> operator*() { return *this->value; }

        void operator++() { this->value += 1; }

        Bool operator!=(const Ptr_Iterator<T>& other) const {
            return this->value < other.value;
        }
    };


    template <typename T>
    struct Range {
        T _begin;
        T _end;

        Range()               : _begin(0),     _end(0)   {}
        Range(T end)          : _begin(0),     _end(end) {}
        Range(T begin, T end) : _begin(begin), _end(end) {}

        T length() const {
            return (this->_end > this->_begin)
                ?  (this->_end - this->_begin)
                :  T(0);
        }

        Monotonic_Iterator<T> begin() const { return Monotonic_Iterator<T> { this->_begin }; }
        Monotonic_Iterator<T> end()   const { return Monotonic_Iterator<T> { this->_end   }; }
    };

}

