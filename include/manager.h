#ifndef _MANAGER_H
#define _MANAGER_H

#include <thread>
#include <vector>
#include <mutex>
#include <fstream>

#include "downloader.h"
#include "node.h"

class Manager
{
	vector<Node*> node_vector;

	public:
	int new_download(addr_struct addr, int num_of_trds);
	void wait();
	virtual void on_status_changed(int downloader_trd_index, size_t total_trd_len, size_t received_bytes,
			addr_struct* addr_data) {};
	virtual void on_get_file_stat(size_t node_index, size_t file_size ,addr_struct* addr_data) {};
};

#endif
