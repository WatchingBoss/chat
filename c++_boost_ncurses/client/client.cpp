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

#define SERVER_NAME "192.168.43.122"
#define DELAY 1000

using string_ptr   = std::shared_ptr<std::string>;
using mesQueue_ptr = std::shared_ptr< std::queue<string_ptr> >;

/* GLOBAL VARIABLES */
static boost::system::error_code er_code;
static mesQueue_ptr mesQueue(new std::queue<string_ptr>);
static bool RUN_CHAT = true;
/* END GLOBAL VARIABLES */

/* 
 * TODO: Create ncurses TUI for client 
 */

static std::string *
getNick()
{
	std::string nick;
	std::cout << "Enter you Nick_Name to prompting in chat" << std::endl;
	std::getline(std::cin, nick);
	if(nick.empty())
		sys_er("getPrompt: cannot save nick_name");
	std::transform(nick.begin(), nick.end(), nick.begin(), ::tolower);
	std::string *prompt = new std::string(nick);
	return prompt;
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
writeToSocket(tcp::socket *socket, string_ptr nickName)
{
	while(RUN_CHAT)
	{
		std::string userInput, inputBuffer;
		std::getline(std::cin, userInput);

		inputBuffer = *nickName + ": " + userInput;
		addTime(inputBuffer);

		if(!inputBuffer.empty())
			EXCEPT_HANDLE( socket->write_some(boost::asio::buffer(inputBuffer), er_code); );

		if(userInput.find("exit") != std::string::npos)
		{
			std::cout << "Chat client terminated" << std::endl;
			RUN_CHAT = false;
		}
	}
}

inline bool
ownMes(const string_ptr mes, string_ptr nick)
{
	if(mes->find(*nick) != std::string::npos)
		return true;
	return false;
}

static void
displayChat(const string_ptr nick)
{
	while(RUN_CHAT)
	{
		if(!mesQueue->empty())
		{
			const string_ptr cur_mes = mesQueue->front();
			if(!ownMes(cur_mes, nick))
				std::cout << *cur_mes << std::endl;

			mesQueue->pop();
		}
		boost::this_thread::sleep(boost::posix_time::millisec(DELAY));
	}
}

int main()
{
	boost::thread_group threads;
	boost::asio::io_context io;

	string_ptr nickName(getNick());

	tcp::endpoint endpoint(boost::asio::ip::address::from_string(SERVER_NAME), PORT_NUM);

	tcp::socket socket(io);
	connect_socket(socket, endpoint);
	std::cout << "Welcome, " << *nickName << ", to chat" << std::endl;

	threads.create_thread(boost::bind(readFromSocket, &socket));
	threads.create_thread(boost::bind(writeToSocket, &socket, nickName));
	threads.create_thread(boost::bind(displayChat, nickName));

	threads.join_all();

	puts("Buy Buy!");

	return 0;
}
