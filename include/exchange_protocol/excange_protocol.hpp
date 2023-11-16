#pragma once

#include "iface_serialized_data.hpp"
#include <string>

namespace exchange
{
    enum MessageTypes : uint32_t
    {
        HEADER_MSG_TYPE = 0,
        FILE_INFO_REQUEST,
        FILE_INFO_RESPONSE
    };

    enum class FileType : uint8_t
    {
        dir = 0,
        file,
        error = 0xFF
    };

    //-------------------------------------------------------------
    // Базовые структуры добавлять сюда, под выравнивание
    #pragma pack(push, 1)    

    struct HeaderData
    {
        uint32_t datType;
        uint32_t dataLenth;
    };           

    // Файловая информация
    struct FileInfo
    {
        FileInfo() = default;
        FileInfo(FileType _type, uint32_t _size, const std::string &_name)
            : type(_type), size(_size), name(_name) {}

        FileType    type = FileType::dir;
        uint32_t    size = 0;
        std::string name = "";
    };

    #pragma pack(pop)
    //-------------------------------------------------------------

    // Шаблон для простых структур
    // TODO: добавить проверку на тривиальность типа
    template <uint32_t PackType, typename PackClass /*, typename std::enable_if<std::is_trivial<PackClass>::value>::type*/>
    struct SerializableData : public ISerializableData
    {
        SerializableData() : ISerializableData() {}
        SerializableData(const PackClass &data) : data(data) {}
        SerializableData(PackClass &&data) : data(std::move(data)) {}

        static const uint32_t packType = PackType;
        inline uint32_t type() const override final { return packType; }

        inline virtual uint32_t getSerializedLenth() const override final
        {
            return sizeof(PackClass);
        }

        void serialize(std::vector<uint8_t> &result) const override final
        {
            result.resize(result.size() + sizeof(PackClass));
            memcpy(result.data(), &data, sizeof(PackClass));
        }

        void deserialize(const std::vector<uint8_t> &source) override final
        {
            memcpy(&data, source.data(), source.size());
        }

        PackClass data;
    };

    // Уточняющий шаблон для сериализации информации о файлах
    template <uint32_t PackType>
    struct SerializableData<PackType, std::vector<FileInfo> /*, std::enable_if_t<std::true_type::value>*/> : public ISerializableData
    {
        SerializableData() : ISerializableData() {}
        SerializableData(const std::vector<FileInfo> &data) : data(data) {}
        SerializableData(std::vector<FileInfo> &&data) : data(std::move(data)) {}

        static const uint32_t packType = PackType;
        inline uint32_t type() const override final
        {
            return packType;
        }

        inline uint32_t getSerializedLenth() const override final
        {
            uint32_t resLenth = 0;
            const size_t constPartSize = sizeof(FileInfo::type) + sizeof(FileInfo::size) + sizeof(uint16_t);
            for (const auto &info : data)
            {
                resLenth += constPartSize;
                resLenth += info.name.size();
            }

            return resLenth;
        }

        /**
        * Сериализация в вектор.
        * @param Вектор может быть не пустым. Добавляем данные в конец.
        *        Удобно для формирования пакета <header> + <data>     
        */
        void serialize(std::vector<uint8_t> &result) const override final
        {
            size_t offset = result.size();
            result.resize(result.size() + getSerializedLenth());
            
            for (const auto &info : data)
            {
                auto len = sizeof(info.type) + sizeof(info.size);
                memcpy(result.data() + offset, &info, len);
                offset += len;

                uint16_t strSize = static_cast<uint16_t>(info.name.size());
                len = sizeof(strSize);
                memcpy(result.data() + offset, &strSize, len);
                offset += len;

                len = strSize;
                memcpy(result.data() + offset, info.name.data(), len);
                offset += len;
            }
        }

        void deserialize(const std::vector<uint8_t> &source) override final
        {
            size_t offset = 0;
            data.clear();

            while (offset < source.size())
            {
                FileInfo info;

                auto len = sizeof(info.type) + sizeof(info.size);
                memcpy(&info, source.data() + offset, len);
                offset += len;

                uint16_t strSize = 0;
                len = sizeof(strSize);
                memcpy(&strSize, source.data() + offset, len);
                offset += len;

                info.name.resize(strSize);
                len = strSize;
                memcpy(info.name.data(), source.data() + offset, len);
                offset += len;

                data.emplace_back(info);
            }
        }

        std::vector<FileInfo> data;
    };

    
    struct PathRequest
    {
        std::string path;
    };

    // Уточняющий шаблон для запроса информации по пути
    template <uint32_t PackType>
    struct SerializableData<PackType, PathRequest>: public ISerializableData
    {
        SerializableData() : ISerializableData() {}
        SerializableData(const PathRequest &data) : data(data) {}

        static const uint32_t packType = PackType;
        inline uint32_t type() const override final
        {
            return packType;
        }

        inline uint32_t getSerializedLenth() const override final
        {
            return data.path.size();
        }

        void serialize(std::vector<uint8_t> &result) const override final
        {
            size_t offset = result.size();
            result.resize(result.size() + getSerializedLenth());
            memcpy(result.data() + offset, data.path.data(), data.path.size());
        }

        void deserialize(const std::vector<uint8_t> &source) override final
        {
            data.path.resize(source.size());
            memcpy(data.path.data(), source.data() , source.size());
        }

        PathRequest data;
    };

}
