#include <iostream>
#include <string>
#include <thread>
#include <array>
#include <chrono>
#include <queue>
#include <cstring>

#include <ncurses.h>

#include "common.h"
#include "tui.h"

struct window_param{
	int height, width, starty, startx;
} chat_display_param, nick_display_param, user_input_param;

struct MyWindows{
	WINDOW *chat, *nick, *input;
};

/* Global variables */
static dispMes_ptr displayedMes(new std::vector<string_ptr>);
string_ptr userInput(new std::string);
/* END */

WINDOW *
create_new_window(const window_param &win)
{
	WINDOW *new_win;
	new_win = newwin(win.height, win.width, win.starty, win.startx);
	box(new_win, 0, 0);
	wrefresh(new_win);
	return new_win;
}

void
set_param_to_window(window_param *display, window_param *nick, window_param *input)
{
	int	work_height = LINES - static_cast<int>(static_cast<float>(LINES) * 0.02f),
		work_width  = COLS - static_cast<int>(static_cast<float>(COLS) * 0.02f),
		work_starty = static_cast<int>(static_cast<float>(LINES) * 0.01f),
		work_startx = static_cast<int>(static_cast<float>(COLS) * 0.01f);

	display->height = static_cast<int>(static_cast<float>(work_height) * 0.88f);
	display->width = static_cast<int>(static_cast<float>(work_width) * 0.88f);
	display->starty = work_starty;
	display->startx = work_startx;

	nick->height = display->height;
	nick->width = work_width - display->width;
	nick->starty = work_starty;
	nick->startx = display->width;

	input->height = work_height - display->height;
	input->width = work_width;
	input->starty = display->height;
	input->startx = work_startx;
}

static MyWindows
define_windows()
{
	set_param_to_window(&chat_display_param, &nick_display_param, &user_input_param);

	WINDOW *chat = create_new_window(chat_display_param);
	WINDOW *nick = create_new_window(nick_display_param);
	WINDOW *input = create_new_window(user_input_param);

	MyWindows my_wins {chat, nick, input};
	return my_wins;
}

static inline bool
ownMes(const string_ptr mes, string_ptr nick)
{
	if(mes->find(*nick) != std::string::npos)
		return true;
	return false;
}

void
chat_display(WINDOW *win)
{
	int c_height = chat_display_param.height;

	while(RUN_CHAT)
	{
		if(!mesQueue->empty())
		{
			const string_ptr cur_mes = mesQueue->front();
			/* attron */
			if(!ownMes(cur_mes, myNick))
				mvwaddnstr(win, c_height - 1, 1, cur_mes->c_str(), cur_mes->length());
			else
				mvwaddnstr(win, c_height - 1, 1, cur_mes->c_str(), cur_mes->length());
			/* attroff */
			for(size_t i = displayedMes->size() - 1, L = 2; i > 0; --i, ++L)
				mvwaddnstr(win, c_height - L, 1,
						   displayedMes->at(i)->c_str(), displayedMes->at(i)->length() );
			wrefresh(win);

			if(displayedMes->size() >= static_cast<size_t>(c_height))
				displayedMes->erase(displayedMes->begin());
			displayedMes->push_back(cur_mes);
			mesQueue->pop();
		}
		refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
	}
}

void
nick_display(WINDOW *win)
{
	(void)win;
}

void
user_input(WINDOW *win)
{
	int c_height = user_input_param.height;

	while(RUN_CHAT)
	{
		constexpr size_t bufferSize = 512;
		char buffer[bufferSize] = {0};

		mvwgetnstr(win, c_height - 2, 1, buffer, bufferSize);
		userInput->assign(buffer);

		memset(buffer, ' ', bufferSize);
		mvwgetnstr(win, c_height - 2, 1, buffer, bufferSize);

		wrefresh(win);
	}

	(void)win;
}

static void
create_threads(std::array<std::thread, 2> &ths,const MyWindows &wins)
{
	std::thread chat_thread(chat_display, wins.chat);
//	std::thread nick_thread(nick_display, wins.nick);
	std::thread input_thread(user_input, wins.input);
	ths.at(0) = std::move(chat_thread);
//	ths.at(1) = std::move(nick_thread);
	ths.at(1) = std::move(input_thread);
}

void
main_tui()
{
	MyWindows myWins = define_windows();

	initscr();

	std::array<std::thread, 2> threads;
	create_threads(threads, myWins);

	for(std::thread &th : threads)
		if(th.joinable()) th.join();

	endwin();
}

/* 
 * Three windows
 * Get user input
 * Display messages
 * Dispaly chat users -> Send user's nick from server
 */
