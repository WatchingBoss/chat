#include <iostream>
#include <string>
#include <list>
#include <map>
#include <queue>
#include <algorithm>
#include <utility>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "../common.hpp"

#define SMALL_DELAY 100
#define LONG_DELAY  200

using socket_ptr      = std::shared_ptr<tcp::socket>;
using string_ptr      = std::shared_ptr<std::string>;
using client_map      = std::map       <socket_ptr, string_ptr>;
using client_map_ptr  = std::shared_ptr< client_map >;
using client_list_ptr = std::shared_ptr< std::list<socket_ptr> >;
using mesQueue_ptr    = std::shared_ptr< std::queue<client_map_ptr> >;

/* GLOBAL VARIABLES */
static boost::system::error_code er_code;
static boost::mutex mtx;
static client_list_ptr clientList(new std::list<socket_ptr>);
static mesQueue_ptr mesQueue(new std::queue<client_map_ptr>);
static bool RUN_SERVER = true;
/* END GLOBAL VARIABLES */

/* 
TODO: Use boost::system::error_code in all posible boost funcs calls!
 */

void
accepting(boost::asio::io_context *io, tcp::acceptor *acceptor)
{
	puts("Waiting for clients....");


	while(RUN_SERVER)
	{
		socket_ptr clientSocket(new tcp::socket(*io));
		acceptor->accept(*clientSocket);

		puts("New client joined");

		mtx.lock();
		clientList->emplace_back(clientSocket);
		mtx.unlock();

		std::cout << "Total client: " << clientList->size() << std::endl;
	}
}

static inline bool
clientSentExit(string_ptr mes)
{
	if(mes->find("exit") != std::string::npos)
		return true;
	else
		return false;
}

static void
disconnectClient(socket_ptr clSock)
{
	auto pos = std::find(clientList->begin(), clientList->end(), clSock);

	clSock->shutdown(tcp::socket::shutdown_both);
	clSock->close();

	clientList->erase(pos);

	std::cout << "Client disconnected. Total clients: " << clientList->size() << std::endl;
}

/* 
 * TODO: Improve way to check new client's messages
 * 1) Create thread for each client
 * 2) Introduce coroutines and kick off a new coroutine for every new client connection
 */

void
requesting()
{
	constexpr size_t bufferSize = 1024;
	std::size_t readBytes = 0;

	while(RUN_SERVER)
	{
		if(!clientList->empty())
		{
			mtx.lock();

			for(auto &clientSocket : *clientList)
			{
				if(clientSocket->available())
				{
					char readBuffer[bufferSize] = {0};
					EXCEPT_HANDLE(
						readBytes = clientSocket->read_some(
							boost::asio::buffer(readBuffer, bufferSize),
							er_code); );
					string_ptr mes(new std::string(readBuffer, readBytes));

					if(clientSentExit(mes))
					{
						disconnectClient(clientSocket);
						break;
					}

					client_map_ptr clMap(new client_map);
					clMap->insert(std::pair<socket_ptr, string_ptr>(clientSocket, mes));

					mesQueue->push(clMap);

					std::cout << "ChatLog: " << *mes << std::endl;
				}				
			}
			mtx.unlock();
		}
		boost::this_thread::sleep(boost::posix_time::millisec(LONG_DELAY));
	}
}

void
responsing()
{
	while(RUN_SERVER)
	{
		if(!mesQueue->empty())
		{
			client_map_ptr mes = mesQueue->front();

			mtx.lock();

			EXCEPT_HANDLE(
				for(socket_ptr &clSock : *clientList)
					clSock->write_some(boost::asio::buffer(*(mes->begin()->second)),
									   er_code); );
			mesQueue->pop();

			mtx.unlock();
		}
		boost::this_thread::sleep(boost::posix_time::millisec(LONG_DELAY));
	}
}

int main()
{
	boost::asio::io_context io;
	tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), PORT_NUM));

	boost::thread_group threads;

	threads.create_thread(boost::bind(accepting, &io, &acceptor));
	boost::this_thread::sleep(boost::posix_time::millisec(SMALL_DELAY));
	threads.create_thread(requesting);
	boost::this_thread::sleep(boost::posix_time::millisec(SMALL_DELAY));
	threads.create_thread(responsing);
	boost::this_thread::sleep(boost::posix_time::millisec(SMALL_DELAY));

	threads.join_all();

	return 0;
}
