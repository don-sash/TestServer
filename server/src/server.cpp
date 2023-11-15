#include "server.hpp"

void Connection::doReadHeader()
{
    auto self = shared_from_this();
    auto headerPtr = std::make_shared<exchange::HeaderData>();
    asio::async_read(socket_, asio::buffer(headerPtr.get(), sizeof(exchange::HeaderData)),
                     [this, self, headerPtr](std::error_code ec, std::size_t len)
                     {
                         if (!ec)
                         {
                             std::cout << "doReadHeader() thread_id: " << std::this_thread::get_id() << std::endl;
                             if (headerPtr->dataLenth)
                                 doReadBody(headerPtr);
                             else
                                 doReadHeader();
                         }
                         else
                         {
                             std::cerr << "Error while reading header: " << ec.message() << std::endl;
                         }
                     });
}

void Connection::doReadBody(std::shared_ptr<exchange::HeaderData> headerPtr)
{
    auto self = shared_from_this();
    auto packageDataPtr = std::make_shared<std::vector<uint8_t>>(headerPtr->dataLenth);
    asio::async_read(socket_, asio::buffer(packageDataPtr->data(), headerPtr->dataLenth),
                     [this, self, packageDataPtr, headerPtr](std::error_code ec, std::size_t len)
                     {
                         if (!ec)
                         {
                             std::cout << "doReadBody() thread_id: " << std::this_thread::get_id() << std::endl;
                             switch (headerPtr->datType)
                             {
                             case exchange::FILE_INFO_REQUEST:
                             {
                                 exchange::SerializableData<exchange::FILE_INFO_REQUEST, exchange::PathRequest> pack;
                                 pack.deserialize(*packageDataPtr);

                                 std::vector<exchange::FileInfo> dirInfoVec;

                                 if (!cache->getValue(pack.data.path, dirInfoVec))
                                 {
                                     dirInfoVec = FileUtils::getDirInfo(pack.data.path);
                                     cache->setValue(pack.data.path, dirInfoVec);
                                 }

                                 auto response = std::make_shared<std::vector<uint8_t>>();

                                 exchange::SerializableData<exchange::FILE_INFO_RESPONSE, std::vector<exchange::FileInfo>> packResponse(std::move(dirInfoVec));
                                 exchange::SerializableData<exchange::HEADER_MSG_TYPE, exchange::HeaderData> packHead;
                                 packHead.data.datType = packResponse.type();
                                 packHead.data.dataLenth = packResponse.getSerializedLenth();
                                 packHead.serialize(*response);
                                 packResponse.serialize(*response);

                                 doWrite(response);
                             }
                             break;

                             default:
                                 std::cout << "Unknown message type: " << headerPtr->datType << std::endl;
                                 break;
                             }
                         }
                         else
                             std::cerr << "Error while reading body: " << ec.message() << std::endl;
                     });
}

void Connection::doWrite(std::shared_ptr<std::vector<uint8_t>> &message)
{
    auto self = shared_from_this();
    asio::async_write(socket_, asio::buffer(*message, message->size()),
                      [this, self, message](std::error_code ec, std::size_t lenght)
                      {
                          if (!ec)
                          {
                              std::cout << "doWrite() thread_id: " << std::this_thread::get_id() << std::endl;
                              doReadHeader();
                          }
                          else
                              std::cerr << "Error while writing response: " << ec.message() << std::endl;
                      });
}