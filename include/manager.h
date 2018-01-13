#ifndef _MANAGER_H
#define _MANAGER_H
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include "downloader.h"
#include "node.h"
#include <fstream>

using namespace std;

class Manager{
	vector<Node*> node_vector;

	public:
	Manager();
	~Manager();
	int new_download(addr_info addr, int num_of_trds);
	void wait();
	void on_status_changed(string prm)
	{
	}

	static void status_changed(void* ptr_to_bject, string text);

};

#endif
