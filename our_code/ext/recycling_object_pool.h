#pragma once

#include <cassert>
#include <type_traits>
#include <vector>

#include "boost/pool/object_pool.hpp"

namespace lost {

// An object pool that recycles its objects instead of deallocating.
// This is a wrapper around `boost::object_pool` to avoid the expensive deallocation
// that happens when `free` is called.
//
// Only works with trivially destructible types, as otherwise destructors of allocated objects may be called multiple times.
//
// `T` is the type of objects allocated from this pool.
// `UserAllocator` is the allocator used by the `boost::object_pool`.
template<typename T, typename UserAllocator = boost::default_user_allocator_new_delete>
    requires std::is_trivially_destructible_v<T>
class recycling_object_pool {
    using object_type = T;
    using pointer_type = object_type*;
    using reference_type = object_type&;
    using user_allocator_type = UserAllocator;

public:
    // Construct a new `recycling_object_pool`.
    // `arg_next_size` and `arg_max_size` are the arguments to the constructor of the underlying `object_pool`.
    //
    // `arg_next_size`: size of the next allocated block
    // `arg_max_size`: maximum block size, `arg_max_size == 0` means unbounded block size
    recycling_object_pool(const size_t arg_next_size = 32, const size_t arg_max_size = 0) :
        pool(arg_next_size, arg_max_size) {}

    // Construct an instance of type `T` with `Args` passed to the constructor.
    // Returns a pointer to the newly constructed instance.
    template<typename... Args>
    pointer_type construct(Args &&...args) {
        if (free_objects.empty()) {
            return pool.construct(args...);
        } 
        // If there is a free object lying around, recycle it.
        auto result = free_objects.back();
        free_objects.pop_back();
        return new(result) object_type{args...};
    }

    // "Frees" the object at `ptr`.
    void free(pointer_type ptr) {
        assert(ptr != nullptr); // Can't free something that doesn't exist
        free_objects.push_back(ptr);
    }

private:
    boost::object_pool<object_type, user_allocator_type> pool;
    std::vector<pointer_type> free_objects;

};

} // End of namespace `lost`
