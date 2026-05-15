#pragma once

#include <cstdint>

enum class BlockType : uint8_t
{
    Air   = 0,
    Grass = 1,
    Sand  = 2,
    Dirt  = 3,
    Stone = 4,
    Count = 5
};

static constexpr uint8_t BLOCK_TYPE_COUNT = 5;
