#ifndef _DOWNLOADER_H
#define _DOWNLOADER_H

#include <iostream>
#include <thread>
#include <fstream>
#include <thread.h>
#include <mutex>
#include <cstdio>

using namespace std;


class Downloader:public Thread{
	private:
		void run();

	protected:
		virtual void downloader_trd() 	= 0;
		function<void(off_t,string,int)> init_callback_func;
	
		int 		index 		= 0;
		//ofstream	*file_pointer 	= NULL;
		FILE*		fd		= NULL;
		off_t		trd_len		= 0;
		off_t		recieved_bytes	= 0;
		off_t		pos		= 0;
		mutex*		file_mutex;
		void *node;



	public:
		Downloader();
		~Downloader();

		int get_index();
		void set_index(int value);

		off_t get_trd_len();

		void set_node_mutex_and_fd(FILE *fd, mutex* file_mutex);
		virtual off_t get_size() = 0;

		void write_to_file(off_t pos, off_t len, char* buf);

};

#endif
