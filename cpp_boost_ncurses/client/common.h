#ifndef CLIENT_COMMON_H
#define CLIENT_COMMON_H

#include <string>
#include <queue>
#include <list>
#include <memory>

#define DELAY 1000

using string_ptr   = std::shared_ptr<std::string>;
using mesQueue_ptr = std::shared_ptr< std::queue<string_ptr> >;
using dispMes_ptr  = std::shared_ptr< std::list<string_ptr> >;

extern mesQueue_ptr mesQueue;
extern bool RUN_CHAT;
extern string_ptr myNick;
extern string_ptr userInput;

#endif
