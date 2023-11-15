#pragma once

#include <cstdint>
#include <vector>
#include <type_traits>
#include <string.h>

namespace exchange
{
    struct ISerializableData
    {
        virtual ~ISerializableData() {}

        virtual uint32_t type() const = 0;
        virtual uint32_t getSerializedLenth() const = 0;
        virtual void serialize(std::vector<uint8_t> &result) const = 0;
        virtual void deserialize(const std::vector<uint8_t> &source) = 0;
    };
}