#pragma once

#include <lpp/core/basic.hpp>
#include <lpp/core/basic_type_info.hpp>
#include <lpp/common/iteration.hpp>
#include <lpp/memory/allocator.hpp>
#include <lpp/memory/buffer_utils.hpp>


namespace lpp {

    struct LPP_API Raw_List {
        static constexpr Usize default_capacity = 16;

        Ptr<Allocator> allocator;
        Ptr<Basic_Type_Info> type_info;

        Addr values   = nullptr;
        Usize     length   = 0;
        Usize     capacity = 0;


        // set default values of fields.
        Raw_List() = default;

        LPP_MOVE_IS_DESTROY_CTORS(Raw_List, Raw_List);


        Void _destroy();

        inline Addr _get_element(Usize index, Usize element_size) const {
            return buffer::get_element(this->values, this->length, index, element_size);
        }
        inline Addr _get_element_unchecked(Usize index, Usize element_size) const {
            return buffer::get_element_unchecked(this->values, index, element_size);
        }


        Range<Usize> get_range() const { return Range<Usize>(this->length); }

        Void set_length(Usize new_length, Bool grow = true);
        Void set_capacity(Usize new_capacity);
        Void reserve(Usize min_capacity, Bool grow = true);

        Void _append_empty();
        Void _append(Addr value);
    };


    template <typename T>
    struct List : Raw_List {
        List(Ptr<Allocator> allocator LPP_USE_DEFAULT_ALLOCATOR) {
            this->allocator = allocator;
            this->type_info = Basic_Type_Info::get<T>();
        }

        LPP_MOVE_IS_DESTROY_CTORS(List, LPP_PASS(List<T>));


        Ref<      T> get(Usize index)                 { return *Ptr<      T>(this->_get_element(index, sizeof(T))); }
        Ref<const T> get(Usize index)           const { return *Ptr<const T>(this->_get_element(index, sizeof(T))); }
        Ref<      T> get_unchecked(Usize index)       { return *Ptr<      T>(this->_get_element_unchecked(index, sizeof(T))); }
        Ref<const T> get_unchecked(Usize index) const { return *Ptr<const T>(this->_get_element_unchecked(index, sizeof(T))); }

        Ref<      T> first()                 { return this->get(0); }
        Ref<const T> first()           const { return this->get(0); }
        Ref<      T> first_unchecked()       { return this->get_unchecked(0); }
        Ref<const T> first_unchecked() const { return this->get_unchecked(0); }
        Ref<      T> last()                  { return this->get(this->length - 1); }
        Ref<const T> last()            const { return this->get(this->length - 1); }
        Ref<      T> last_unchecked()        { return this->get_unchecked(this->length - 1); }
        Ref<const T> last_unchecked()  const { return this->get_unchecked(this->length - 1); }

#if LPP_DEBUG
        Ref<      T> operator[](Usize index)       { return this->get(index); }
        Ref<const T> operator[](Usize index) const { return this->get(index); }
#else
        Ref<      T> operator[](Usize index)       { return this->get_unchecked(index); }
        Ref<const T> operator[](Usize index) const { return this->get_unchecked(index); }
#endif


        Ptr<      T> get_values()       { return Ptr<      T>(this->values); }
        Ptr<const T> get_values() const { return Ptr<const T>(this->values); }


        Ref<T> append_empty() {
            this->_append_empty();
            return this->last_unchecked();
        }

        Ref<T> append(Ptr<T> value) {
            this->_append(Addr(value));
            return this->last_unchecked();
        }

        template <typename ...Args>
        Ref<T> append_new(Args... args) {
            auto& value = this->append_empty();
            new (&value) T(lpp_forward(args)...);
            return value;
        }

        RRef<T> pop() {
            if(this->length == 0) {
                throw "List has no entries.";
            }

            this->length -= 1;
            return lpp_move(*Ptr<T>(this->_get_element_unchecked(this->length, sizeof(T))));
        }

        Ptr_Iterator<      T> begin()       { return Ptr_Iterator<      T>(Ptr<      T>(this->_get_element_unchecked(0,            sizeof(T)))); }
        Ptr_Iterator<      T> end()         { return Ptr_Iterator<      T>(Ptr<      T>(this->_get_element_unchecked(this->length, sizeof(T)))); }
        Ptr_Iterator<const T> begin() const { return Ptr_Iterator<const T>(Ptr<const T>(this->_get_element_unchecked(0,            sizeof(T)))); }
        Ptr_Iterator<const T> end()   const { return Ptr_Iterator<const T>(Ptr<const T>(this->_get_element_unchecked(this->length, sizeof(T)))); }
    };


    namespace proto {
        LPP_MOVE_IS_DESTROY(LPP_PASS(typename T), LPP_PASS(List<T>));

        LPP_IMPL_PROTO(Destroy, LPP_PASS(typename T), LPP_PASS(List<T>), LPP_PASS(
            [](Addr object) {
                Ptr<Raw_List>(object)->_destroy();
            },
        ));
    }

}

