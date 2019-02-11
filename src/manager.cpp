#include "manager.h"
#include "http_downloader.h"
#include <unistd.h>
#include "node.h"

using namespace std;

int Manager::new_download(addr_struct addr, int num_of_trds)
{

	Node *nd = new Node(addr, num_of_trds, this);
	nd->start();
	node_vector.push_back(nd);
	return node_vector.size();
}

void Manager::wait()
{
	for (std::vector<Node*>::iterator it = node_vector.begin();
			it != node_vector.end(); ++it){
		(*it)->join();
		delete (*it);
	}
}
