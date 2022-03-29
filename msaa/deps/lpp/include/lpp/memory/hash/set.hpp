#pragma once

#include <lpp/core/basic.hpp>
#include <lpp/memory/allocator.hpp>
#include <lpp/memory/hash/container.hpp>

namespace lpp {

    struct LPP_API Raw_Hash_Set {
        using Key_Info = Hash_Container::Key_Info;

        Ptr<Key_Info>  key_info;
        Hash_Container container;


        Void _destroy();

        Void rehash();

        Void set_capacity(U32 new_capacity);

        Hash_Container::Insert _insert(Addr key);

        Bool _remove_swap(Addr key);

        Max_Opt<U32> _query(Addr key);
    };



    template <typename T>
    struct Hash_Set : Raw_Hash_Set {
        Hash_Set(Ptr<Allocator> allocator LPP_USE_DEFAULT_ALLOCATOR) {
            static Key_Info key_info = Key_Info::get<T>();

            this->key_info = &key_info;
            this->container.allocator = allocator;
        }

        Hash_Set(RRef<Hash_Set<T>> other) { move(this, &other); }
        Ref<Hash_Set<T>> operator=(RRef<Hash_Set<T>> other) { move(this, &other); return *this; }

        // Hash_Set cannot be copied.
        Hash_Set(Ref<Hash_Set<T>> other) = delete;
        Ref<Hash_Set<T>> operator=(Ref<Hash_Set<T>>) = delete;


        Hash_Container::Insert insert_maybe(Ptr<T> key) {
            return this->_insert(Addr(key));
        }

        template <typename ...Args>
        Bool insert_maybe(Args... args) {
            auto key = T(lpp_forward(args)...);
            auto inserted = this->insert_maybe(&key).was_new;
            if(_not(inserted)) {
                destroy_maybe(&key);
            }
            return inserted;
        }

        Bool remove_maybe_swap(Ref<const T> key) {
            return this->_remove_swap(Addr(&key));
        }

        Max_Opt<U32> query(Ref<const T> key) {
            return this->_query(Addr(&key));
        }

        Bool contains(Ref<const T> key) {
            return this->query(key).is_some();
        }
    };


    namespace proto {
        LPP_MOVE_IS_DESTROY(LPP_PASS(typename T), LPP_PASS(Hash_Set<T>));

        LPP_IMPL_PROTO(Destroy, LPP_PASS(typename T), LPP_PASS(Hash_Set<T>), LPP_PASS(
            [](Addr object) {
                Ptr<Raw_Hash_Set>(object)->_destroy();
            },
        ));
    }

}

