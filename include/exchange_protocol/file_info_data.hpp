
#pragma once

#include "iface_serialized_data.hpp"
#include <string>


//namespace FileData
// {

#pragma pack(push, 1)

enum class FileType : uint8_t
{
    dir = 0,
    file,    
    error = 0xFF
};


struct FileInfo
{
    FileInfo()  = default;
    FileInfo(FileType _type, uint32_t _size, const std::string& _name)
        : type(_type), size(_size), name(_name){}
    FileType    type;
    uint32_t    size = 0;
    std::string name = "";
};

#pragma pack(pop)

template <uint32_t PackType>
struct SerializableData< PackType, std::vector<FileInfo>/*, std::enable_if_t<std::true_type::value>*/> : public ISerializableData
{
    SerializableData():ISerializableData(){}
    SerializableData(const std::vector<FileInfo> &data): data(data){}
    SerializableData(std::vector<FileInfo> &&data): data(std::move(data)){}


    static const uint32_t packType = PackType;
    inline uint32_t type() const override final
    {
        return packType;
    }

    inline uint32_t getSerializedLenth() const override final
    {
        uint32_t resLenth = 0;
        const size_t constPartSize = sizeof(FileInfo::type)  + sizeof(FileInfo::size) + sizeof(uint16_t);
        for(const auto& info : data)
        {
            resLenth += constPartSize;
            resLenth += info.name.size();
        }

        return resLenth;
    }

    void serialize(std::vector<uint8_t> &result) const override final
    {
        result.resize(getSerializedLenth());
        size_t offset = 0;
        for(const auto& info : data)
        {
            auto len = sizeof(info.type) + sizeof(info.size);
            memcpy(result.data() + offset, &info, len); offset += len;

            uint16_t strSize = static_cast<uint16_t>(info.name.size());
            len = sizeof(strSize);
            memcpy(result.data() + offset, &strSize, len); offset += len;

            len = strSize;
            memcpy(result.data() + offset, info.name.data(), len); offset += len;
        }
    }

    void deserialize(const std::vector<uint8_t> &source) override final
    {
        size_t offset = 0;
        data.clear();

        while(offset < source.size())
        {
            FileInfo info;

            auto len = sizeof(info.type) + sizeof(info.size);
            memcpy(&info, source.data() + offset, len);  offset += len;

            uint16_t strSize = 0;
            len = sizeof(strSize);
            memcpy(&strSize, source.data() + offset, len);  offset += len;

            info.name.resize(strSize);
            len = strSize;
            memcpy(info.name.data(), source.data() + offset, len);  offset += len;

            data.emplace_back(std::move(info));
        }
    }

    std::vector<FileInfo> data;
};


// }

