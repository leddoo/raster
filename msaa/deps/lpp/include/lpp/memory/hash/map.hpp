#pragma once

#include <lpp/core/basic.hpp>
#include <lpp/memory/allocator.hpp>
#include <lpp/memory/hash/container.hpp>

namespace lpp {

    struct LPP_API Raw_Hash_Map {
        using Key_Info = Hash_Container::Key_Info;

        struct Type_Info {
            Key_Info        key;
            Basic_Type_Info value;
        };


        Ptr<Type_Info> type_info;
        Addr           values = nullptr;
        Hash_Container container;


        Void _destroy();

        Void rehash();

        Void set_capacity(U32 new_capacity);

        Bool _insert(Addr key, Addr value);
        Bool _remove_swap(Addr key, Opt_Addr value_out);
        Bool _query(Addr key, Opt_Ptr<U32> key_index_out);


        Void _adjust_value_buffer(U32 old_capacity, U32 old_length);
    };


    template <typename K, typename V>
    struct Hash_Map : Raw_Hash_Map {
        Hash_Map(Ptr<Allocator> allocator LPP_USE_DEFAULT_ALLOCATOR) {
            static Type_Info type_info = {
                Key_Info::get<K>(),
                *Basic_Type_Info::get<V>(),
            };

            this->type_info = &type_info;
            this->container.allocator = allocator;
        }

        Hash_Map(RRef<Hash_Map<K, V>> other) { move(this, &other); }
        Ref<Hash_Map<K, V>> operator=(RRef<Hash_Map<K, V>> other) { move(this, &other); return *this; }

        // Hash_Map cannot be copied.
        Hash_Map(Ref<Hash_Map<K, V>> other) = delete;
        Ref<Hash_Map<K, V>> operator=(Ref<Hash_Map<K, V>>) = delete;


        Bool insert_maybe(Ptr<K> key, Ptr<V> value) {
            return this->_insert(Addr(key), Addr(value));
        }

        Bool insert_maybe(K key, V value) {
            auto inserted = this->insert_maybe(&key, &value);
            if(_not(inserted)) {
                destroy_maybe(&key);
                destroy_maybe(&value);
            }
            return inserted;
        }

        Bool remove_maybe_swap(Ref<const K> key) {
            return this->_remove_swap(Addr(&key), nullptr);
        }


        Bool contains(Ref<const K> key) {
            return this->_query(Addr(&key));
        }
    };


    namespace proto {
        LPP_MOVE_IS_DESTROY(LPP_PASS(typename K, typename V), LPP_PASS(Hash_Map<K, V>));

        LPP_IMPL_PROTO(Destroy, LPP_PASS(typename K, typename V), LPP_PASS(Hash_Map<K, V>), LPP_PASS(
            [](Addr object) {
                Ptr<Raw_Hash_Map>(object)->_destroy();
            },
        ));
    }

}

