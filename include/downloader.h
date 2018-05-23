#ifndef _DOWNLOADER_H
#define _DOWNLOADER_H

#include <iostream>
#include <thread>
#include <fstream>
#include <thread.h>
#include <mutex>
#include <cstdio>
#include "definitions.h"

using namespace std;


class Downloader:public Thread{
	private:
		void run();
		string log_buffer_str;
		long size = 0;
		bool is_start_pos_written = false;

	protected:
		virtual void downloader_trd() 	= 0;
		function<void(off_t,string,int)> init_callback_func;
		constexpr static size_t MAX_HTTP_HEADER_LENGTH = 64 * 1024;
		virtual bool regex_search_string(string& input, string& output) {};

		int 		index 		= 0;
	//	FILE*		fp		= NULL;	//main file descriptor.
	//	FILE*		log_fp		= NULL;	//log file descriptor.
		off_t		trd_len		= 0;	// file size in bytes
		off_t		recieved_bytes	= 0;	
		char*		log_buffer	= NULL;
		off_t		pos		= 0;	//fp last position
		void *node;

		node_struct*	node_data;
		addr_struct	addr_data;

		void write_to_file(off_t pos, off_t len, char* buf);
		void write_log_file(off_t pos);
		void write_start_pos_log(off_t start_pos);
		
	public:
		Downloader(node_struct* node_data, const addr_struct, off_t pos,
				off_t trd_length, int index);
		~Downloader();

		int get_index();
		void set_index(int value);

		off_t get_trd_len();

		virtual off_t get_size() = 0;
		virtual bool check_redirection(string& redirect_url) {};

		/**
		 * Check the size of file and redirection
		 *
		 * @param redirect_url: Will be filled with redirected url if
		 * 		redirection exist.
		 * @param size: Will be filled with size of file if exist.
		 *
		 * @return True if redirection detected
		 */
		virtual bool check_link(string& redirect_url, off_t& size) {};
};

#endif
