#include "common.hpp"

#include <iostream>

void sys_er(const char *er) {
	std::cout << er << std::endl;
	exit(EXIT_FAILURE);
}

void sys_er(boost::system::error_code e_c) {
	std::cout << "Code: " << e_c << "=> " << e_c.message() << std::endl;
	exit(EXIT_FAILURE);
}
