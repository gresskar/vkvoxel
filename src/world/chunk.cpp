#include "world/chunk.hpp"

#include <stdexcept>

Chunk::Chunk(ChunkCoord coord)
    : m_coord(coord)
{
    m_blocks.fill(BlockType::Air);
}

const ChunkCoord &Chunk::coord(void) const
{
    return m_coord;
}

int32_t Chunk::minBlockX(void) const
{
    return m_coord.x * static_cast<int32_t>(WIDTH);
}

int32_t Chunk::minBlockZ(void) const
{
    return m_coord.z * static_cast<int32_t>(DEPTH);
}

BlockType Chunk::get(uint32_t x, uint32_t y, uint32_t z) const
{
    if (x >= WIDTH || y >= HEIGHT || z >= DEPTH)
    {
        throw std::out_of_range("Chunk::get(): local block coordinate is out of range");
    }

    return m_blocks.at(index(x, y, z));
}

void Chunk::set(uint32_t x, uint32_t y, uint32_t z, BlockType block)
{
    if (x >= WIDTH || y >= HEIGHT || z >= DEPTH)
    {
        throw std::out_of_range("Chunk::set(): local block coordinate is out of range");
    }

    m_blocks.at(index(x, y, z)) = block;
    m_isDirty = true;
}

bool Chunk::dirty(void) const
{
    return m_isDirty;
}

void Chunk::markDirty(void)
{
    m_isDirty = true;
}

void Chunk::clearDirty(void)
{
    m_isDirty = false;
}

size_t Chunk::index(uint32_t x, uint32_t y, uint32_t z)
{
    return static_cast<size_t>(x) + static_cast<size_t>(WIDTH) * (static_cast<size_t>(z) + static_cast<size_t>(DEPTH) * static_cast<size_t>(y));
}
