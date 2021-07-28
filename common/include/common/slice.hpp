#pragma once

#include "common.hpp"

template <typename T>
struct Slice {
    T* values;
    Uint count;

    T& operator[](Uint index) {
        assert(index < count);
        return this->values[index];
    }

    const T& operator[](Uint index) const {
        assert(index < count);
        return this->values[index];
    }
};


template <typename T>
struct Slice_Iterator {
    T* value;

    T& operator*() const { return *this->value; }

    void operator++() { this->value += 1; }

    Bool operator!=(const Slice_Iterator<T>& other) const {
        return this->value != other.value;
    }
};

template <typename T>
Slice_Iterator<T> begin(const Slice<T>& slice) {
    return Slice_Iterator<T> { slice.values };
}

template <typename T>
Slice_Iterator<T> end(const Slice<T>& slice) {
    return Slice_Iterator<T> { slice.values + slice.count };
}


template <typename T>
Slice<T> make_slice(T* values, Uint count) {
    auto result = Slice<T> {};
    result.values = values;
    result.count  = count;
    return result;
}

template <typename T>
Slice<T> make_slice(T* begin, T* end) {
    assert(begin <= end);
    auto result = Slice<T> {};
    result.values = begin;
    result.count  = (Uint)(end - begin);
    return result;
}

template <typename T>
Slice<T> make_slice(T* values, Uint count, Uint begin, Uint end) {
    assert(
           begin <= count
        && end   <= count
        && begin <= end
    );
    return make_slice(values + begin, end - begin);
}

template <typename T>
Slice<T> make_slice_from(T* values, Uint count, Uint begin) {
    assert(begin <= count);
    return make_slice(values + begin, count - begin);
}

template <typename T>
Slice<T> make_slice_until(T* values, Uint count, Uint end) {
    assert(end <= count);
    return make_slice(values, end);
}


template <typename T>
Slice<T> make_slice(Slice<T> slice) {
    return slice;
}

template <typename T>
Slice<T> make_slice(Slice<T> slice, Uint begin, Uint end) {
    return make_slice(slice.values, slice.count, begin, end);
}

template <typename T>
Slice<T> make_slice_from(Slice<T> slice, Uint begin) {
    return make_slice_from(slice.values, slice.count, begin);
}

template <typename T>
Slice<T> make_slice_until(Slice<T> slice, Uint end) {
    return make_slice_until(slice.values, slice.count, end);
}


template <typename T, Uint n>
Slice<T> make_slice(T (&array)[n]) {
    return make_slice(&array[0], n);
}


template <typename T>
Slice<T> make_slice(List<T>& list) {
    return make_slice(list.data(), list.size());
}

template <typename T>
Slice<T> make_slice(List<T>& list, Uint begin, Uint end) {
    return make_slice(list.data(), list.size(), begin, end);
}

template <typename T>
Slice<T> make_slice_from(List<T>& list, Uint begin) {
    return make_slice_from(list.data(), list.size(), begin);
}

template <typename T>
Slice<T> make_slice_until(List<T>& list, Uint end) {
    return make_slice_until(list.data(), list.size(), end);
}



template <typename T, Uint n>
Slice<T> make_slice(Array<T, n>& array) {
    return make_slice(array.data(), n);
}

template <typename T, Uint n>
Slice<T> make_slice(Array<T, n>& array, Uint begin, Uint end) {
    return make_slice(array.data(), n, begin, end);
}

template <typename T, Uint n>
Slice<T> make_slice_from(Array<T, n>& array, Uint begin) {
    return make_slice_from(array.data(), n, begin);
}

template <typename T, Uint n>
Slice<T> make_slice_until(Array<T, n>& array, Uint end) {
    return make_slice_until(array.data(), n, end);
}

