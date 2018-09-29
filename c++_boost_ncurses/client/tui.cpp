#include <cstring>
#include <ctime>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <array>
#include <list>
#include <queue>

#include <ncurses.h>

#include "common.h"
#include "tui.h"

struct window_param {
	int height, width, starty, startx;
} chat_display_param, nick_display_param, user_input_param;

struct MyWindows {
	WINDOW *chat, *nick, *input;
};

/* Global variables */
static dispMes_ptr displayedMes(new std::list<string_ptr>);
string_ptr         userInput(new std::string);
/* END */

WINDOW *create_new_window(const window_param &win) {
	WINDOW *new_win;
	new_win = newwin(win.height, win.width, win.starty, win.startx);
	box(new_win, 0, 0);
	wrefresh(new_win);
	return new_win;
}

void set_param_to_window(window_param *display, window_param *nick,
                         window_param *input) {
	int work_height =
	        LINES - static_cast<int>(static_cast<float>(LINES) * 0.02f),
	    work_width  = COLS - static_cast<int>(static_cast<float>(COLS) * 0.02f),
	    work_starty = static_cast<int>(static_cast<float>(LINES) * 0.01f),
	    work_startx = static_cast<int>(static_cast<float>(COLS) * 0.01f);

	display->height = static_cast<int>(static_cast<float>(work_height) * 0.88f);
	display->width  = static_cast<int>(static_cast<float>(work_width) * 0.88f);
	display->starty = work_starty;
	display->startx = work_startx;

	nick->height = display->height;
	nick->width  = work_width - display->width;
	nick->starty = work_starty;
	nick->startx = display->width;

	input->height = work_height - display->height;
	input->width  = work_width;
	input->starty = display->height;
	input->startx = work_startx;
}

static MyWindows define_windows() {
	set_param_to_window(&chat_display_param, &nick_display_param,
	                    &user_input_param);

	WINDOW *chat  = create_new_window(chat_display_param);
	WINDOW *nick  = create_new_window(nick_display_param);
	WINDOW *input = create_new_window(user_input_param);

	MyWindows my_wins{ chat, nick, input };
	return my_wins;
}

static inline bool ownMes(const string_ptr mes, string_ptr nick) {
	if (mes->find(*nick) != std::string::npos) return true;
	return false;
}

static void cleanWindow(WINDOW *win, const window_param &param) {
	const int     height = param.height;
	constexpr int width  = 100;
	char          buffer[width];

	memset(buffer, ' ', width);

	for (int i = 1; i < param.height; ++i)
		mvwaddnstr(win, height - i, 1, buffer, width);
}

/* 
 * TODO: Why SEGFAULT???
 */

void chat_display(WINDOW *win) {
	int height = chat_display_param.height;

	while (RUN_CHAT) {
		if (!mesQueue->empty()) {
			cleanWindow(win, chat_display_param);

			int line = height - 1;
			const size_t dispMesCount = displayedMes->size();
			const string_ptr new_mes = mesQueue->front();
			/* attron */
			if (!ownMes(new_mes, myNick))
				mvwaddnstr(win, line, 1, new_mes->c_str(),
				           new_mes->length());
			else
				mvwaddnstr(win, line, 1, new_mes->c_str(),
				           new_mes->length());
			/* attroff */
			if(dispMesCount)
				for(const string_ptr &str : *displayedMes)
					mvwaddnstr(win, ++line, 1, str->c_str(), str->length());
			if (dispMesCount >= static_cast<size_t>(height))
				displayedMes->pop_front();

			displayedMes->push_back(new_mes);
			mesQueue->pop();
		}
		wrefresh(win);
		std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
	}
}

void nick_display(WINDOW *win) { (void)win; }

void user_input(WINDOW *win) {
	const int height = user_input_param.height;
	const int promptLine = height - 2;

	while (RUN_CHAT) {
		constexpr size_t bufferSize         = 512;
		char             buffer[bufferSize] = { 0 };

		mvwgetnstr(win, promptLine, 1, buffer, bufferSize);
		userInput->assign(buffer);

		cleanWindow(win, user_input_param);

		wrefresh(win);
	}
}

static void create_threads(std::array<std::thread, 2> &ths,
                           const MyWindows &           wins) {
	std::thread chat_thread(chat_display, wins.chat);
	//	std::thread nick_thread(nick_display, wins.nick);
	std::thread input_thread(user_input, wins.input);
	ths.at(0) = std::move(chat_thread);
	//	ths.at(1) = std::move(nick_thread);
	ths.at(1) = std::move(input_thread);
}

void main_tui() {
	MyWindows myWins = define_windows();

	initscr();

	std::array<std::thread, 2> threads;
	create_threads(threads, myWins);

	for (std::thread &th : threads)
		if (th.joinable()) th.join();

	endwin();
}

/*
 * Three windows
 * Get user input
 * Display messages
 * Dispaly chat users -> Send user's nick from server
 */
