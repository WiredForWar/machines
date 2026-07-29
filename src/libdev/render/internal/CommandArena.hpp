#pragma once

#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>

namespace Ren
{

// Scratch storage for command payloads that must outlive the caller, i.e. the
// vertex data of a queued bufferData command.
//
// Chunked rather than a single buffer because commands hold raw pointers into
// it: growing must never move what was already handed out. reset() rewinds
// without freeing, so a steady-state frame does no allocation at all.
class CommandArena
{
public:
    static constexpr std::size_t MinChunkBytes = 64 * 1024;

    // Copies sizeBytes from data into the arena and returns a pointer to the
    // copy, valid until the next reset().
    void* append(const void* data, std::size_t sizeBytes)
    {
        if (sizeBytes == 0)
            return nullptr;

        Chunk* chunk = chunkWithRoomFor(sizeBytes);
        std::byte* at = chunk->data.get() + chunk->used;
        std::memcpy(at, data, sizeBytes);
        chunk->used += sizeBytes;
        return at;
    }

    // Rewinds every chunk, keeping the memory for the next frame.
    void reset()
    {
        for (Chunk& chunk : chunks_)
            chunk.used = 0;
        current_ = 0;
    }

    std::size_t capacity() const
    {
        std::size_t total = 0;
        for (const Chunk& chunk : chunks_)
            total += chunk.capacity;
        return total;
    }

private:
    struct Chunk
    {
        std::unique_ptr<std::byte[]> data{};
        std::size_t capacity{};
        std::size_t used{};
    };

    Chunk* chunkWithRoomFor(std::size_t sizeBytes)
    {
        while (current_ < chunks_.size())
        {
            Chunk& chunk = chunks_[current_];
            if (chunk.capacity - chunk.used >= sizeBytes)
                return &chunk;

            // A partly used chunk cannot fit this payload; move on rather than
            // splitting it, since payloads have to stay contiguous.
            ++current_;
        }

        const std::size_t capacity = sizeBytes > MinChunkBytes ? sizeBytes : MinChunkBytes;
        chunks_.push_back(Chunk{ std::make_unique<std::byte[]>(capacity), capacity, 0 });
        current_ = chunks_.size() - 1;
        return &chunks_.back();
    }

    std::vector<Chunk> chunks_{};
    std::size_t current_{};
};

} // namespace Ren
