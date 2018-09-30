#include <iostream>
#include <string>
#include <array>
#include <queue>
#include <algorithm>
#include <ctime>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "../common.hpp"
#include "common.h"
#include "tui.h"

#define SERVER_NAME "192.168.43.122"

/* GLOBAL VARIABLES */
static boost::system::error_code er_code;
mesQueue_ptr mesQueue(new std::queue<string_ptr>);
bool RUN_CHAT = true;
string_ptr myNick(new std::string);
/* END GLOBAL VARIABLES */

/* 
 * TODO: Create ncurses TUI for client 
 */

static std::string
getNick()
{
	std::string nick;
	std::cout << "Enter you Nick_Name to prompting in chat" << std::endl;
	std::getline(std::cin, nick);
	if(nick.empty())
		sys_er("getPrompt: cannot save nick_name");
	std::transform(nick.begin(), nick.end(), nick.begin(), ::tolower);
	return nick;
}

static void
connect_socket(tcp::socket &socket, tcp::endpoint &ep)
{
	EXCEPT_HANDLE( socket.connect(ep, er_code); );
}

static void
readFromSocket(tcp::socket *socket)
{
	constexpr size_t bufferSize = 1024;
	std::size_t readBytes = 0;

	while(RUN_CHAT)
	{
		EXCEPT_HANDLE(
			if(socket->available(er_code))
			{
				char readBuffer[bufferSize] = {0};
				readBytes = socket->read_some(boost::asio::buffer(readBuffer, bufferSize),
											  er_code);
				string_ptr mes(new std::string(readBuffer, readBytes));
				mesQueue->push(mes);
			} );
		boost::this_thread::sleep(boost::posix_time::millisec(DELAY));
	}
}

static void
addTime(std::string &input)
{
	char buf[15] = {0};
	time_t c_time = time(NULL);
	struct tm *c_time_info = localtime(&c_time);
	strftime(buf, 10, "%H:%M:%S => ", c_time_info);
	input = buf + input;
}

static void
writeToSocket(tcp::socket *socket)
{
	while(RUN_CHAT)
	{
		if(userInput->empty())
			continue;
		std::string inputBuffer;

		inputBuffer = *myNick + ": " + *userInput;
		addTime(inputBuffer);

		if(!inputBuffer.empty())
			EXCEPT_HANDLE( socket->write_some(boost::asio::buffer(inputBuffer), er_code); );

		if(userInput->find("exit") != std::string::npos)
		{
			string_ptr term(new std::string("Chat terminating"));
			mesQueue->push(term);
			
			RUN_CHAT = false;
		}

		userInput->clear();
	}
}

int main()
{
	boost::thread_group threads;
	boost::asio::io_context io;

	myNick->assign(getNick());

	tcp::endpoint endpoint(boost::asio::ip::address::from_string(SERVER_NAME), PORT_NUM);

	tcp::socket socket(io);
	connect_socket(socket, endpoint);

	mesQueue->push((string_ptr)new std::string("Welcome to chat, " + *myNick));

	threads.create_thread(main_tui);

	threads.create_thread(boost::bind(readFromSocket, &socket));
	threads.create_thread(boost::bind(writeToSocket, &socket));

	threads.join_all();

	std::puts("Chat terminated\nBuy Buy!");

	return 0;
}
