#ifndef _NODE_H
#define _NODE_H

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include "downloader.h"
#include "thread.h"
#include <fstream>
#include "definitions.h"
#include <map>
using namespace std;

class Node:public Thread {

	public:
	Node(const addr_struct , int number_of_trds, void* pointer_to_manager);
	void Display(const string text) { cout << text << endl; cout<<"this="<<this<<endl; };
	static void wrapper_to_get_status(void* ptr_to_object, int downloader_trd_index, off_t received_bytes, int stat_flag=0);

	void call_manager_stat_change(void* ptr_to_object, void (*ptr_to_function)(void* ptr_to_object, string text))
	{
		ptr_to_function(ptr_to_object,"--");  // make callback
	}


	private:
	node_struct*	node_data;

	map<int, Downloader*> download_threads;
	map<int, Downloader*>::iterator download_threads_it;

	map<int, int> start_positions;
	map<int, int> stopped_positions;
	map<int, off_t> trds_length;

	addr_struct dwl_str;

	int 	num_of_trds; 
	off_t 	file_length = 0;
	off_t 	total_received_bytes = 0;
	float 	progress = 0;
	float	speed = 0; // bytes/sec

	int 	index = 0; // index of node
	void 	*ptr_to_manager;

	void wait();
	void run();
	void get_status(int downloader_trd_index, off_t received_bytes, int stat_flag);
	void check_url_details();
	bool read_resume_log();

};

#endif
