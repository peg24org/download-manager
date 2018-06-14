#ifndef _NODE_H
#define _NODE_H

#include <map>

#include "thread.h"
#include "downloader.h"
#include "definitions.h"

using namespace std;

class Node:public Thread {

	public:
	Node(const addr_struct , int number_of_trds, void* pointer_to_manager);
	void Display(const string text) { cout << text << endl; cout<<"this="<<this<<endl; };
	static void wrapper_to_get_status(void* ptr_to_object, int downloader_trd_index, size_t received_bytes, int stat_flag=0);

	void call_manager_stat_change(void* ptr_to_object, void (*ptr_to_function)(void* ptr_to_object, string text))
	{
		ptr_to_function(ptr_to_object,"--");  // make callback
	}


	private:
	node_struct*	node_data;

	map<int, Downloader*> download_threads;
	map<int, Downloader*>::iterator download_threads_it;

	map<size_t, size_t> start_positions;
	map<size_t, size_t> stopped_positions;
	map<size_t, size_t> trds_length;

	addr_struct dwl_str;

	int 	num_of_trds; 
	size_t 	file_length = 0;
	size_t 	total_received_bytes = 0;
	float 	progress = 0;
	float	speed = 0; // bytes/sec

	int 	index = 0; // index of node
	void 	*ptr_to_manager;

	void wait();
	void run();
	void get_status(int downloader_trd_index, size_t received_bytes, int stat_flag);
	void check_url_details();
	void  read_resume_log();
	void check_file_exist();
};

#endif
