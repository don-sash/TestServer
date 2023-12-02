
#include <iostream>

#include <unistd.h>
#include <boost/asio.hpp>

#include "./../../include/exchange_protocol/excange_protocol.hpp"
#include "./../../include/file_utils.hpp"

using boost::asio::ip::tcp;

namespace asio = boost::asio;

int main(int argc, char *argv[])
{
    std::string host = "127.0.0.1";
    uint16_t    port = 5071;

    std::cout << "************* Start TCP client *************" << std::endl;

    std::cout << "Usage: Run with args: client <host> <port>" << std::endl;
    std::cout << "  or with defaults host = " << host << " port = " << port << std::endl << std::endl;              

    if (argc > 1)
    {
        std::istringstream iss(argv[1]);
        if (!(iss >> host))
        {
            std::cout << "Wrong host argument!";
            return 1;
        }
    }

    if (argc > 2)
    {
        std::istringstream iss(argv[2]);
        if (!(iss >> port))
        {
            std::cout << "Wrong port argument!";
            return 1;
        }
    }

    exchange::HeaderData header;
    exchange::PathRequestPack packRequest;
    exchange::FileInfoPack packResponse;    

    std::vector<uint8_t> requestBuf;
    std::vector<uint8_t> responseBuf;

    while (true)
    {
        try
        {
            // Создаем объекты для соединения с сервером и отправки данных
            asio::io_context io_context;
            tcp::socket socket(io_context);
            tcp::resolver resolver(io_context);
            asio::connect(socket, resolver.resolve(host, std::to_string(port)));

            std::cout << "Connection to server " << host << ":" << port << " established!" << std::endl;

            while (true)
            {
                std::cout << "Enter path or \"Enter\" for exit: ";
                std::getline(std::cin, packRequest.data.path);
                if (packRequest.data.path.size() == 0)
                    return 0;

                // Формируем структуру запроса
                header.datType   = packRequest.type();
                header.dataLenth = static_cast<uint32_t>(packRequest.getSerializedLenth());

                // Отправляем головок
                asio::write(socket, asio::buffer(&header, sizeof(header)));

                // Отправляем запрос
                requestBuf.clear();
                packRequest.serialize(requestBuf);
                asio::write(socket, asio::buffer(requestBuf));

                // Получаем заголовок
                asio::read(socket, asio::buffer(&header, sizeof(header)));

                // Получаем тело
                responseBuf.resize(header.dataLenth);
                asio::read(socket, asio::buffer(responseBuf, header.dataLenth));

                if (header.datType == exchange::FILE_INFO_RESPONSE)
                {
                    packResponse.deserialize(responseBuf);
                    FileUtils::printFileInfoVec(packResponse.data);
                }
            }
        }
        catch (std::exception &e)
        {
            std::cerr << "Error connecting to " << host << ":" << port << std::endl
                      << "e.what()->" << e.what() << std::endl;
            return 1;
        }
    }

    return 0;
}
