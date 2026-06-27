#pragma once
#include <vector>
#include <cstdint>
#include <cassert>
#include "ecs/Entity.hpp"

namespace rts{

// Common base so the registry can hold many SparseSet<T> of different types in one array
struct ISparseSet{
    virtual ~ISparseSet() = default;
    virtual void remove_if_present(Entity e) = 0;
};

template <typename T>
class SparseSet : public ISparseSet {
public:
    static constexpr std::uint32_t INVALID = 0xFFFFFFFFu;

    bool has(Entity e) const{
        std::uint32_t i = entity_index(e);
        return (i < sparse.size()) && (sparse[i] != INVALID);
    }

    T& add(Entity e, T value){
        std::uint32_t i = entity_index(e);
        if (i >= sparse.size()) 
            sparse.resize(i + 1, INVALID);
        assert(!has(e) && "component already present");
        sparse[i] = dense.size();
        dense.push_back(value);
        dense_to_entity.push_back(i);
        return dense.back();
    }

    void remove(Entity e){
        assert(has(e));
        std::uint32_t i = entity_index(e);
        std::uint32_t slot = sparse[i];
        dense[slot] = dense.back();
        dense_to_entity[slot] = dense_to_entity.back();
        sparse[dense_to_entity[slot]] = slot;
        dense.pop_back();
        dense_to_entity.pop_back();
        sparse[i] = INVALID;
    }

    // called by the registry on entity destruction, it is safe even if absent
    void remove_if_present(Entity e) override {
        if (has(e)) remove(e);
    }

    T& get(Entity e){
         assert(has(e)); 
         return dense[sparse[entity_index(e)]];
    }

    std::size_t size() const{ 
        return dense.size(); 
    }

    // raw access for systems to iterate
    std::vector<T>& data(){ 
        return dense; 
    }
    std::vector<std::uint32_t>& entity_indices(){ 
        return dense_to_entity; 
    }

private:
    std::vector<T> dense;
    std::vector<std::uint32_t> dense_to_entity;
    std::vector<std::uint32_t> sparse;
};

}