#include <iostream>
#include <boost/asio.hpp>

#include "../common.hpp"

#define SERVER_NAME "192.168.43.122"

void get_er(system::error_code er_code)
{
	std::cout << er_code << ": " << er_code.message() << std::endl;
	exit(EXIT_FAILURE);
}

int main()
{
	asio::io_context io;

	tcp::resolver resolver(io);
	tcp::resolver::results_type endpoint = resolver.resolve(SERVER_NAME, PORT_NUM_STR);

	tcp::socket socket(io);
	asio::connect(socket, endpoint);

	/* 
	 *TODO: Make func to get line
	 Do it assync
	*/

	for(;;)
	{
		std::array<char, 256> mes;
		std::string out;
		system::error_code er;

		std::cin >> out;

		asio::write(socket, asio::buffer(out), er);

		size_t len = socket.read_some(asio::buffer(mes), er);

		if(er && er != asio::error::eof)
			get_er(er);

		if(len)
			std::cout << mes.data() << std::endl;
	}

	return 0;
}
