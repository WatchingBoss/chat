#ifndef COMMON_HPP
#define COMMON_HPP

#include <boost/asio.hpp>

using namespace boost::asio::ip;

#define PORT_NUM 8888
#define PORT_NUM_STR "8888"

extern void sys_er(const char *);
extern void sys_er(boost::system::error_code);

#define EXCEPT_HANDLE(x)						\
	try{x}										\
	catch(std::exception &e){					\
		std::cout << e.what() << std::endl;		\
		sys_er(er_code);						\
	}

#endif
