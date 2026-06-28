#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include "ecs/Entity.hpp"
#include "ecs/SparseSet.hpp"

namespace rts {

// counter fr types
inline std::uint32_t next_type_id() {
    static std::uint32_t counter = 0;
    return counter++;
}

// gives each component type a fixed number. Position gets 0, Velocity 1, etc
// the static only runs once per type
template <typename T>
std::uint32_t type_id(){
    static std::uint32_t id = next_type_id();
    return id;
}

class Registry{
public:
    Entity create(){
        // reuse old slot if available
        if (!free_indices.empty()){
            std::uint32_t index = free_indices.back();
            free_indices.pop_back();
            return make_entity(index, generations[index]);
        }
        // nothing free, make a new slot
        std::uint32_t index = generations.size();
        generations.push_back(0);
        return make_entity(index, 0);
    }

   bool valid(Entity e) const{
        std::uint32_t i = entity_index(e);
        // mask the counter to 12 bits so it matches the handles generation field
        // entity_generation already returns just those 12 bits
        return i < generations.size() && (generations[i] & 0xFFFu) == entity_generation(e);
    }

    void destroy(Entity e){
        if (!valid(e)) return;
        std::uint32_t i = entity_index(e);
        for (auto& pool : pools)
            if (pool) pool->remove_if_present(e);
        generations[i]++;            // bump gen so old handles go stale
        free_indices.push_back(i);   
    }

    template <typename T>
    T& add(Entity e, T value) {
        return pool<T>().add(e, value);
    }

    template <typename T>
    void remove(Entity e) {
        pool<T>().remove(e);
    }

    template <typename T>
    bool has(Entity e) {
        return pool<T>().has(e);
    }

    template <typename T>
    T& get(Entity e) {
        return pool<T>().get(e);
    }

    // runs fn on everything that has both A and B
    template <typename A, typename B, typename Fn>
    void view(Fn fn) {
        SparseSet<A>& pa = pool<A>();
        SparseSet<B>& pb = pool<B>();

        // copy the list first, otherwise if fn adds/remove stuff
        std::vector<std::uint32_t> indices = pa.entity_indices();

        for (std::uint32_t i : indices){
            Entity e = make_entity(i, generations[i]);
            if (!pb.has(e)) continue; //no B, skip it
            fn(e, pa.get(e), pb.get(e));
        }
    }

    // grabs the pool for type T,makes it the first time if needed
    template <typename T>
    SparseSet<T>& pool(){
        std::uint32_t id = type_id<T>();
        if (id >= pools.size())
            pools.resize(id + 1);
        if (!pools[id])
            pools[id] =std::make_unique<SparseSet<T>>();
        return *static_cast<SparseSet<T>*>(pools[id].get());
    }

private:
    std::vector<std::uint32_t> generations; // how many times each slot got reused
    std::vector<std::uint32_t> free_indices; // dead slots waiting to be reused
    std::vector<std::unique_ptr<ISparseSet>> pools; //one pool per component type
};

} 