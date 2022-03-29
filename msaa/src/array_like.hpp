#pragma once


#define lpp_array_like_def_accessors                                        \
    Void check_bounds(Usize index) const {                                  \
        if(index >= n) {                                                    \
            throw "Index out of bounds.";                                   \
        }                                                                   \
    }                                                                       \
    \
    Void check_bounds_debug(Usize index) const {                            \
        if(LPP_DEBUG) {                                                     \
            this->check_bounds(index);                                      \
        }                                                                   \
    }                                                                       \
    \
    Ref<      T> at(Usize index)                 { this->check_bounds(index); return this->values()[index]; } \
    Ref<const T> at(Usize index)           const { this->check_bounds(index); return this->values()[index]; } \
    Ref<      T> at_unchecked(Usize index)       { this->check_bounds_debug(index); return this->values()[index]; } \
    Ref<const T> at_unchecked(Usize index) const { this->check_bounds_debug(index); return this->values()[index]; } \
    \
    Ref<      T> first()                 { return this->at(0); } \
    Ref<const T> first()           const { return this->at(0); } \
    Ref<      T> first_unchecked()       { return this->at_unchecked(0); } \
    Ref<const T> first_unchecked() const { return this->at_unchecked(0); } \
    Ref<      T> last()                  { return this->at(this->length() - 1); } \
    Ref<const T> last()            const { return this->at(this->length() - 1); } \
    Ref<      T> last_unchecked()        { return this->at_unchecked(this->length() - 1); } \
    Ref<const T> last_unchecked()  const { return this->at_unchecked(this->length() - 1); } \
    \
    Ref<      T> operator[](Usize index)       { return this->at_unchecked(index); } \
    Ref<const T> operator[](Usize index) const { return this->at_unchecked(index); } \
    \
    Ptr<      T> begin()       { return &this->values()[0]; } \
    Ptr<const T> begin() const { return &this->values()[0]; } \
    Ptr<      T> end()         { return &this->values()[this->length()]; } \
    Ptr<const T> end()   const { return &this->values()[this->length()]; } \
    \
    Range<Usize> range() const { return Range<Usize>(this->length()); }

