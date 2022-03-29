#pragma once

#include <lpp/core/basic.hpp>
#include <lpp/core/basic_type_info.hpp>
#include <lpp/memory/allocator.hpp>

namespace lpp {
namespace buffer {

    /* buffer
        - utilities for working with Array<T, n> where T or n are dynamic.
    */

    // SECTION: allocation.

    inline Addr allocate(Usize capacity, Usize size, Usize alignment, Ptr<Allocator> allocator LPP_USE_DEFAULT_ALLOCATOR) {
        return Addr(allocator->allocate(capacity*size, alignment));
    }

    inline Addr allocate(Usize capacity, Ptr<Basic_Type_Info> type_info, Ptr<Allocator> allocator LPP_USE_DEFAULT_ALLOCATOR) {
        return allocate(capacity, type_info->size, type_info->alignment, allocator);
    }

    template <typename T>
    Ptr<T> allocate(Usize capacity, Ptr<Allocator> allocator LPP_USE_DEFAULT_ALLOCATOR) {
        return Ptr<T>(allocate(capacity, sizeof(T), alignof(T), allocator));
    }


    // SECTION: free with element destroy.

    inline Void free(Ref<Addr> buffer, Usize length, Ptr<Basic_Type_Info> type_info, Ptr<Allocator> allocator LPP_USE_DEFAULT_ALLOCATOR) {
        destroy_array(type_info, buffer,  0, length);
        allocator->safe_free(buffer);
    }


    // SECTION: element access.

    inline Addr get_element_unchecked(Addr buffer, Usize index, Usize element_size) {
        return buffer + index*element_size;
    }

    inline Addr get_element_unchecked(Addr buffer, Usize index, Ptr<Basic_Type_Info> type_info) {
        return get_element_unchecked(buffer, index, type_info->size);
    }

    inline Addr get_element(Addr buffer, Usize length, Usize index, Usize element_size) {
        if(index >= length) {
            throw "Index out of bounds.";
        }
        return get_element_unchecked(buffer, index, element_size);
    }

    inline Addr get_element(Addr buffer, Usize length, Usize index, Ptr<Basic_Type_Info> type_info) {
        return get_element(buffer, length, index, type_info->size);
    }


    // SECTION: set capacity, ignoring values.

    inline Void set_capacity_empty(
        Ref<Addr> buffer, Usize new_capacity,
        Usize size, Usize alignment,
        Ptr<Allocator> allocator LPP_USE_DEFAULT_ALLOCATOR
    ) {
        allocator->safe_free(buffer);

        if(new_capacity > 0) {
            buffer = allocate(new_capacity, size, alignment, allocator);
        }
    }

    inline Void set_capacity_empty(Ref<Addr> buffer, Usize new_capacity, Ptr<Basic_Type_Info> type_info, Ptr<Allocator> allocator LPP_USE_DEFAULT_ALLOCATOR) {
        set_capacity_empty(buffer, new_capacity, type_info->size, type_info->alignment, allocator);
    }

    template <typename T>
    Void set_capacity_empty(Ref<Ptr<T>> buffer, Usize new_capacity, Ptr<Basic_Type_Info> type_info, Ptr<Allocator> allocator LPP_USE_DEFAULT_ALLOCATOR) {
        set_capacity_empty(reinterpret_cast<Ref<Addr>>(buffer), new_capacity, type_info, allocator);
    }


    // SECTION: set capacity change, move and destroy elements.

    // returns the new length = at_most(new_capacity, old_length).
    LPP_API Usize set_capacity_move(
        Ref<Addr> buffer, Usize old_length, Usize new_capacity,
        Ptr<Basic_Type_Info> type_info, Ptr<Allocator> allocator LPP_USE_DEFAULT_ALLOCATOR
    );

    template <typename T>
    Usize set_capacity_move(Ref<Ptr<T>> buffer, Usize old_length, Usize new_capacity, Ptr<Basic_Type_Info> type_info, Ptr<Allocator> allocator LPP_USE_DEFAULT_ALLOCATOR) {
        return set_capacity_move(reinterpret_cast<Ref<Addr>>(buffer), old_length, new_capacity, type_info, allocator);
    }


    // SECTION: swap removal.

    // - if value_out.is_some, moves dst into value_out.value, else destroys dst.
    // - if src_index != dst_index, moves src into dst.
    LPP_API Void remove_swap_unchecked(
        Addr buffer, Usize dst_index, Usize src_index,
        Opt_Addr value_out,
        Ptr<Basic_Type_Info> type_info
    );

    inline Void remove_swap(
        Addr buffer, Usize length, Usize dst_index, Usize src_index,
        Opt_Addr value_out,
        Ptr<Basic_Type_Info> type_info
    ) {
        if(dst_index >= length || src_index >= length) {
            throw "Index out of bounds.";
        }
        remove_swap_unchecked(buffer, dst_index, src_index, value_out, type_info);
    }

}}

