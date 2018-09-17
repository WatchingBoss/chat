#include <iostream>
#include <string>

#include <boost/asio.hpp>

#include "../common.hpp"

int main()
{
	asio::io_context io;

	tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), PORT_NUM));

	for(;;)
	{
		std::array<char, 256> mes;

		tcp::socket socket(io);
		acceptor.accept(socket);

		system::error_code er;

		size_t len = socket.read_some(asio::buffer(mes), er);

		if(len)
			asio::write(socket, asio::buffer(mes), er);
	}

	return 0;
}
