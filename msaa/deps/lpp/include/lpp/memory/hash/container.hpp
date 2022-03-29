#pragma once

#include <lpp/core/basic.hpp>
#include <lpp/core/basic_type_info.hpp>
#include <lpp/common/equal.hpp>
#include <lpp/memory/allocator.hpp>
#include <lpp/memory/hash/hash.hpp>

namespace lpp {

    /* Hash_Container
        - A dynamic hash set. can be used to implement hash maps.
        - Keys must implement hash and equal.
        - Uses 32 bit hashes. maximum capacity is 2^31.
        - Uses internal chaining with quadratic probing and 2^n table sizes.
        - Keys are stored separately and in insertion order (remove affects this).
    */
    struct LPP_API Hash_Container {
        static constexpr U32 default_capacity = 32;

        static constexpr U32 max_load_factor_numerator   = 13;
        static constexpr U32 max_load_factor_denominator = 16;

        static constexpr U32 get_max_key_count(U32 capacity) {
            return (capacity * max_load_factor_numerator) / max_load_factor_denominator;
        }

        struct Slot {
            static constexpr U32 fresh     = U32(-1);
            static constexpr U32 tombstone = U32(-2);
            static constexpr U32 min_special_value = tombstone;

            U32 key_index;

            Bool has_entry() const { return this->key_index < min_special_value; }
            Bool is_empty()  const { return _not(this->has_entry()); }
        };

        struct Key_Info : Basic_Type_Info {
            Ptr<proto::Hash_Record>  hash;
            Ptr<proto::Equal_Record> equal;

            template <typename T>
            static Key_Info get() {
                auto result = Key_Info();
                static_cast<Basic_Type_Info&>(result) = *Basic_Type_Info::get<T>();
                result.hash = proto::Hash<T, U32>::get();
                result.equal = proto::Equal<T>::get();
                return result;
            }
        };



        Ptr<Allocator> allocator = nullptr;
        Ptr<Slot>      slots     = nullptr;
        Addr           keys      = nullptr;
        U32            length    = 0;
        U32            capacity  = 0;


        Void _destroy(Ptr<Key_Info> key_info);

        /* rehash
            - Recompute the hash slots.
            - This removes any tombstones.
            - Call this after making a lot of removals to speed up searching.
        */
        Void rehash(Ptr<Key_Info> key_info);

        /* set_capacity
            - New_capacity must be a power of 2.
            - Set the size of the hash slot array to new_capacity.
            - Note: the number of supported keys is lower (see get_max_key_count).
        */
        Void set_capacity(U32 new_capacity, Ptr<Key_Info> key_info);


        /* search
            - Try to find a key by using its hash value.
            - If the key is found: returns the index of the slot containing the
                index of the corresponding key.
              Else: returns the index of the slot where the key's index can be
                written if the key is to be inserted into the set.
        */
        struct Search {
            U32 slot_index;
        };
        Search search(U32 hash_value, Addr key, Ptr<Key_Info> key_info);

        /* Search_key_index
            - For a given key present in the set, find its slot index given the
              key's hash value and index in the key array.
            - `first_empty`: the first unused slot in the key's slot chain, or the
              key's slot index itself if there is no unused slot before it.
        */
        Search search_key_index(U32 hash_value, U32 key_index, Ref<U32> first_empty);


        /* insert
            - Try to insert a key into the set.
            - `key` is moved into the key array if `key` was not present before
              (`was_new` = true).
            - Returns the index of the key in the key array.
        */
        struct Insert {
            U32  key_index;
            Bool was_new;
        };
        Insert insert(Addr key, Ptr<Key_Info> key_info);


        /* remove
            - Try to remove a key from the set.
            - `key` is not modified.
            - If `key` is found:
                - Corresponding key in the key array is destroyed.
                - If destroyed key was not last in the key array, the last key
                  is swapped into key's place.
                - Returns the index of the key in the key array before removal.
              else:
                - Returns none.
        */
        struct Remove {
            Max_Opt<U32> key_index;
        };
        Remove remove(Addr key, Ptr<Key_Info> key_info);


        /* query
            - Search for a key in the set.
            - Returns the index of the key in the key array if found.
        */
        struct Query {
            Max_Opt<U32> key_index;
        };
        Query query(Addr key, Ptr<Key_Info> key_info);
    };

}

