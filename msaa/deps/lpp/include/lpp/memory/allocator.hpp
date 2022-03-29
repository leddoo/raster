#pragma once

#include <lpp/core/basic.hpp>
#include <lpp/core/proto.hpp>

namespace lpp {

    namespace proto {

        /* allocator protocol
            - Define for memory allocator types.
            - `allocate` must return a pointer aligned to `alignment`.
              this may require the internal allocation to be larger than size.
              Currently, there is no way for the user to know the exact size of
              their allocation.
            - `free` is optional. This can be used for "batch free" allocators
              like arenas.
        */
        struct Allocator_Record {
            using allocate_Proc = Addr(*)(Addr self, Usize size, Usize alignment);
            using free_Opt_Proc = Void(*)(Addr self, Addr allocation);

            allocate_Proc allocate;
            free_Opt_Proc free;
        };

        LPP_DEF_PROTO(Allocator, LPP_PASS(typename T), LPP_PASS(T));

    }


    LPP_API Addr allocate(Ptr<proto::Allocator_Record> record, Addr self, Usize size, Usize alignment);
    LPP_API Void free    (Ptr<proto::Allocator_Record> record, Addr self, Addr allocation);


    template <typename T>
    Void safe_free(Ptr<proto::Allocator_Record> record, Addr self, Ref<Ptr<T>> allocation) {
        if(allocation != nullptr) {
            free(record, self, Addr(allocation));
            allocation = nullptr;
        }
    }


    template <typename Allocator>
    Addr allocate(Ref<Allocator> allocator, Usize size, Usize alignment) {
        allocate(proto::Allocator<Allocator>::get(), Addr(&allocator), size, alignment);
    }

    template <typename T, typename Allocator>
    Ptr<T> allocate(Ref<Allocator> allocator) {
        return Ptr<T>(allocate(allocator, sizeof(T), alignof(T)));
    }

    template <typename Allocator>
    Void free(Ref<Allocator> allocator, Addr allocation) {
        free(proto::Allocator<Allocator>::get(), Addr(&allocator), allocation);
    }

    template <typename Allocator, typename T>
    Void safe_free(Ref<Allocator> allocator, Ref<Ptr<T>> allocation) {
        safe_free(proto::Allocator<Allocator>::get(), Addr(&allocator), allocation);
    }


    /* Allocator
        - use pointers to this type for v-table based allocation.
    */
    struct LPP_API Allocator {
        Ptr<proto::Allocator_Record> vtable;
        // T wrapped; <- put here.

        Addr get_wrapped_addr() const { return Addr(&this->vtable) + sizeof(this->vtable); }


        Addr allocate(Usize size, Usize alignment) {
            return ::lpp::allocate(this->vtable, this->get_wrapped_addr(), size, alignment);
        }

        Void free(Addr allocation) {
            ::lpp::free(this->vtable, this->get_wrapped_addr(), allocation);
        }

        template <typename T>
        Ptr<T> allocate() {
            return Ptr<T>(this->allocate(sizeof(T), alignof(T)));
        }

        template <typename T>
        Void safe_free(Ref<Ptr<T>> allocation) {
            ::lpp::safe_free(this->vtable, this->get_wrapped_addr(), allocation);
        }
    };

    /* default_allocator.
        - lpp optionally supports using an allocator as the global default.
        - lpp never uses the default_allocator internally. It only uses it for
          default arguments to make usage more concise in the presence of a
          default allocator.
        - If used, but not defined, a linker error will be raised.
          This can make it hard to find accidental usage. Sadly I don't know of
          a way around this issue.
    */
    #ifdef LPP_HAS_DEFAULT_ALLOCATOR
        extern Ptr<Allocator> default_allocator;
        #define LPP_USE_DEFAULT_ALLOCATOR = default_allocator
    #else
        #define LPP_USE_DEFAULT_ALLOCATOR
    #endif


    /* Allocator Wrapper
        - Wrap an allocator instance for v-table based allocation.
        - Enables using instances of allocator types with Ptr<Allocator>.
    */
    template <typename T>
    struct Allocator_Wrapper : Allocator {
        static_assert(meta::are_equal<T, meta::Remove_Reference<T>>(), "T must not be a reference.");

        T wrapped;

        template <typename ...Args>
        Allocator_Wrapper(Args... args)
            : wrapped(lpp_forward(args)...)
        {
            this->vtable = proto::Allocator<T>::get();
            static_assert(offsetof(Allocator_Wrapper, wrapped) == sizeof(Allocator::vtable), "Unsupported alignment.");
        }

        LPP_MOVE_IS_DESTROY_CTORS(Allocator_Wrapper, LPP_PASS(Allocator_Wrapper<T>));
    };

}

