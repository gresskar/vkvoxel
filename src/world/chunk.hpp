#pragma once

#include "world/block.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

struct ChunkCoord
{
    int32_t x = 0;
    int32_t z = 0;

    [[nodiscard]] bool operator==(const ChunkCoord &other) const = default;
};

class Chunk
{
public:
    static constexpr uint32_t WIDTH = 32;
    static constexpr uint32_t DEPTH = 32;
    static constexpr uint32_t HEIGHT = 64;
    static constexpr uint32_t BLOCK_COUNT = WIDTH * DEPTH * HEIGHT;

    explicit Chunk(ChunkCoord coord = {});

    [[nodiscard]] const ChunkCoord &coord(void) const;
    [[nodiscard]] int32_t minBlockX(void) const;
    [[nodiscard]] int32_t minBlockZ(void) const;

    [[nodiscard]] BlockType get(uint32_t x, uint32_t y, uint32_t z) const;
    void set(uint32_t x, uint32_t y, uint32_t z, BlockType block);

    [[nodiscard]] bool dirty(void) const;
    void markDirty(void);
    void clearDirty(void);

private:
    [[nodiscard]] static size_t index(const uint32_t x, const uint32_t y, const uint32_t z);

    ChunkCoord m_coord{};
    std::array<BlockType, BLOCK_COUNT> m_blocks{};
    bool m_isDirty = true;
};

template <>
struct std::hash<ChunkCoord>
{
    [[nodiscard]] size_t operator()(const ChunkCoord &coord) const noexcept
    {
        const uint64_t x = static_cast<uint32_t>(coord.x);
        const uint64_t z = static_cast<uint32_t>(coord.z);
        return static_cast<size_t>((x << 32u) ^ z);
    }
};
