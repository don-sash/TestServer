#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include <iostream>
#include <thread>

#include "./../../include/cache.hpp"
#include "./../../include/file_utils.hpp"
#include "./../../include/exchange_protocol/excange_protocol.hpp"


using cacheType = cache::TemporalCache<std::string, std::vector<exchange::FileInfo>>;
using cachePtr  = std::shared_ptr<cacheType>;

namespace asio = boost::asio;

class Connection : public std::enable_shared_from_this<Connection>
{
public:
	Connection(boost::asio::io_context& io_context, cachePtr _cache) 
	 : socket_(io_context), 
	   strand_(io_context), 
	   cache(_cache) 
	{ }

	asio::ip::tcp::socket &socket() { return socket_; }
	
	void start()
	{
		std::cout << "New connection, socket descriptor: " << socket_.native_handle() << std::endl;
		doReadHeader();
	}

private:
	void doReadHeader();
	void doReadBody(std::shared_ptr<exchange::HeaderData> headerPtr);
	void doWrite(std::shared_ptr<std::vector<uint8_t>> &message);

	asio::ip::tcp::socket socket_;
	asio::io_context::strand strand_;
	cachePtr cache;
};


class Server
{
private:
	asio::ip::tcp::acceptor acceptor_;
	asio::io_context &io_context_;
	std::shared_ptr<cacheType> cache_; 
	std::shared_ptr<Connection> new_connection_;

public:
	Server(asio::io_context &io_context, uint16_t port, uint32_t cacheLifeTimeSec = 10) 
	: io_context_(io_context),
	  acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))													
	{
		std::cout << "The TCP server is listening on port " << port << std::endl;
        cache_ = std::make_shared<cacheType>(std::chrono::seconds(cacheLifeTimeSec));
		doAccept();
	}

	void Run(uint32_t threadsCount = 10)
	{		
		// Создаём пул потоков для io_context. Потоки будут назначаться для выполнения асинхронных операций
  		std::vector<std::shared_ptr<std::thread> > threads;
  		for (std::size_t i = 0; i < threadsCount; ++i)
  		{
    		std::shared_ptr<std::thread> thread(new std::thread(
          	std::bind(static_cast<size_t (boost::asio::io_service::*)()>(&boost::asio::io_service::run), &io_context_)));
    		threads.push_back(thread);
			//std::cout << "io_context_ thread_id: " << thread->get_id() << std::endl;
  		}

  		// Ожидаем завершение всех потоков io_context
  		for (std::size_t i = 0; i < threads.size(); ++i)
    		threads[i]->join();
	}

private:
	void doAccept()
	{
		new_connection_.reset(new Connection(io_context_, cache_)); 
		acceptor_.async_accept(new_connection_->socket(), [this](const std::error_code &error)
								{
									if (!error) 			
										new_connection_->start();								 
									else 
										std::cerr << "Error while accepting new connection: " << error.message() << std::endl;

								doAccept(); });
	}

};
