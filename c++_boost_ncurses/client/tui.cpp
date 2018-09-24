#include <iostream>
#include <string>
#include <thread>
#include <array>

#include <ncurses.h>

#include "tui.h"

struct window_param{
	int height, width, starty, startx;
};

struct MyWindows{
	WINDOW *chat, *nick, *input;
};

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
	window_param chat_display_param, nick_display_param, user_input_param;
	set_param_to_window(&chat_display_param, &nick_display_param, &user_input_param);

	WINDOW *chat = create_new_window(chat_display_param);
	WINDOW *nick = create_new_window(nick_display_param);
	WINDOW *input = create_new_window(user_input_param);

	return {chat, nick, input};
}

void
chat_display()
{

}

void
nick_display()
{

}

void
user_input()
{

}

static void
create_threads(std::array<std::thread, 3> &ths, const MyWindows &wins)
{
	std::thread chat_thread(chat_display, wins.chat);
	std::thread nick_thread(nick_display, wins.nick);
	std::thread input_thread(user_input, wins.input);
	ths.at(0) = std::move(chat_thread);
	ths.at(1) = std::move(nick_thread);
	ths.at(2) = std::move(input_thread);

}

void
main_tui()
{
	MyWindows myWins = define_windows();

	initscr();

	std::array<std::thread, 3> threads;
	create_threads(threads, myWins);

	for(std::thread &th : threads)
		if(th.joinable()) th.join();
}

/* 
 * Three windows
 * Get user input
 * Display messages
 * Dispaly chat users -> Send user's nick from server
 */
