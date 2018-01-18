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
	map<int, Downloader*> download_threads;
	map<int, Downloader*>::iterator download_threads_it;

	map<int, int> start_positions;
	map<int, int> stopped_positions;
	map<int, off_t> trds_length;

	addr_struct dwl_str;
	protocol_type protocol;

	int 	num_of_trds = 0; 
	off_t 	file_length = 0;
	off_t 	total_received_bytes = 0;
	float 	progress = 0;
	float	speed = 0; // bytes/sec

	int 	index = 0; // index of node
	void 	*ptr_to_manager;

	void wait();
	void run();
	void get_status(int downloader_trd_index, off_t received_bytes, int stat_flag);
	bool read_resume_log();

	public:
	Node(const addr_struct , int num_of_trds, protocol_type protocol, void* ptr_to_manager);
	void Display(const string text) { cout << text << endl; cout<<"this="<<this<<endl; };
	static void wrapper_to_get_status(void* ptr_to_object, int downloader_trd_index, off_t received_bytes, int stat_flag=0);

	void call_manager_stat_change(void* ptr_to_object, void (*ptr_to_function)(void* ptr_to_object, string text))
	{
		ptr_to_function(ptr_to_object,"--");  // make callback
	}

	node_struct*	node_data;
};

#endif
