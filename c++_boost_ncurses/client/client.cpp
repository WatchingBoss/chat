#include <iostream>
#include <string>
#include <array>
#include <queue>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "../common.hpp"

#define SERVER_NAME "192.168.43.122"

using string_ptr   = boost::shared_ptr<std::string>;
using mesQueue_ptr = boost::shared_ptr< std::queue<string_ptr> >;

/* GLOBAL VARIABLES */
static system::error_code er_code;
static mesQueue_ptr mesQueue(new std::queue<string_ptr>);
static bool RUN_CHAT = true;
/* END GLOBAL VARIABLES */

static void
sys_er(const char *er)
{
	std::cout << er << std::endl;
	exit(EXIT_FAILURE);
}

static void
sys_er(system::error_code e_c)
{
	std::cout << "Code: " << e_c << "=> " << e_c.message() << std::endl;
	exit(EXIT_FAILURE);
}

#define EXCEPT_HANDLE(x) \
	try{x} catch(std::exception &e){std::cout << e.what() << std::endl; sys_er(er_code);}

static std::string *
getNick()
{
	std::string nick;
	std::cout << "Enter you Nick_Name to prompting in chat" << std::endl;
	std::getline(std::cin, nick);
	if(nick.empty())
		sys_er("getPrompt: cannot save nick_name");
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
	constexpr size_t readSize = 1024;
	std::size_t readBytes = 0;
	char readBuffer[readSize] = {0};

	while(RUN_CHAT)
	{
		EXCEPT_HANDLE(
			if(socket->available(er_code))
			{
				readBytes = socket->read_some(asio::buffer(readBuffer), er_code);
				string_ptr mes(new std::string(readBuffer, readBytes));
				mesQueue->push(mes);
			} );
		bzero(readBuffer, readSize);
		this_thread::sleep(posix_time::millisec(1000));
	}
}

static void
writeToSocket(tcp::socket *socket, string_ptr nickName)
{
	std::string userInput, inputBuffer;

	while(RUN_CHAT)
	{
		std::getline(std::cin, userInput);

		if(userInput.find("exit") != std::string::npos)
		{
			std::cout << "Chat client terminated" << std::endl;
			RUN_CHAT = false;
		}

		inputBuffer = *nickName + ": " + userInput + '\n';

		if(!inputBuffer.empty())
			EXCEPT_HANDLE( socket->write_some(asio::buffer(inputBuffer), er_code); );

		userInput.clear();
		inputBuffer.clear();
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
				std::cout << cur_mes << std::endl;
			else
				std::cout << "OWN->" << cur_mes << std::endl;

			mesQueue->pop();
		}
		this_thread::sleep(posix_time::millisec(1000));
	}
}

int main()
{
	boost::thread_group threads;
	asio::io_context io;

	string_ptr nickName(getNick());

	tcp::endpoint endpoint(asio::ip::address::from_string(SERVER_NAME), PORT_NUM);

	tcp::socket socket(io);
	connect_socket(socket, endpoint);
	std::cout << "Welcome, " << nickName << ", to chat" << std::endl;

	threads.create_thread(boost::bind(readFromSocket, &socket));
	threads.create_thread(boost::bind(writeToSocket, &socket, nickName));
	threads.create_thread(boost::bind(displayChat, nickName));

	threads.join_all();

	puts("Buy Buy!");

	return 0;
}
