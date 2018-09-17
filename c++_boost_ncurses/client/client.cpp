#include <iostream>
#include <boost/asio.hpp>

using namespace boost;

int main()
{
	asio::io_context io;

	std::cout << "Client works" << std::endl;

	return 0;
}
