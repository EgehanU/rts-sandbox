#pragma once
#include <cstdint>

namespace rts{
    // An entity is 32 bit id, low 20 for the index
    // High 12 bits is for the generation

    using Entity = std::uint32_t;

    constexpr std::uint32_t ENTITY_INDEX_BITS = 20;
    constexpr std::uint32_t ENTITY_GEN_BITS = 12;

    constexpr Entity NULL_ENTITY = 0xFFFFFFFFu;

    constexpr std::uint32_t ENTITY_INDEX_MASK = 0x000FFFFF;

    constexpr std::uint32_t entity_index(Entity e){
        return (e & ENTITY_INDEX_MASK);
    }
    constexpr std::uint32_t entity_generation(Entity e) {
        return (e >> ENTITY_INDEX_BITS);   // shift discards the index, no mask needed
    }

    // pack an index and a generation into one Entity value.
    // shift generation left into the high bits, or in the masked index
    constexpr Entity make_entity(std::uint32_t index, std::uint32_t generation){
        return (index & ENTITY_INDEX_MASK) | ((generation & 0xFFFu) << ENTITY_INDEX_BITS);
    }

}