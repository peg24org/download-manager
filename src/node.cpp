#include "node.h"
#include <unistd.h>
#include <cstdio>
#include "manager.h"
#include "http_downloader.h"
Node::Node(addr_info dwl_str, int num_of_trds, protocol_type protocol, void *ptr_to_manager)
{
	this->dwl_str = dwl_str;
	this->protocol = protocol;
	this->num_of_trds = num_of_trds;
	this->ptr_to_manager = ptr_to_manager;
}
void Node::wait()
{
	for (map<int, Downloader*>::iterator it=download_threads.begin(); it!=download_threads.end(); ++it){
		it->second->join();
	}
}
void Node::run()
{
	Downloader *dwl = new HttpDownloader(dwl_str, 0, -1,this,0);
	file_length = dwl->get_size();
	delete dwl;
	off_t trd_norm_len = file_length/num_of_trds;
	// Create file with specified size
	ofs = new ofstream(dwl_str.file_name_on_server, ios::binary | ios::out);
	ofs->seekp(file_length-1);
	ofs->write(" ", 1);
	ofs->close();
	FILE*    fp = fopen(dwl_str.file_name_on_server.c_str(), "w");
	Downloader *dnwl;
	switch(protocol){
		case http_1_0:
			break;
		case http_1_1:
			for(int i=0; i<num_of_trds; i++){
				off_t len = trd_norm_len;
				off_t pos = i*trd_norm_len;
				if(i == (num_of_trds-1)){
					len = file_length-(trd_norm_len*i);
					pos = i*trd_norm_len;
				}
				dnwl = new HttpDownloader(dwl_str, pos,len-1, this,i);
				dnwl->set_node_mutex_and_fd(fp, &node_file_mutex);
				dnwl->start();
				download_threads[i] = dnwl;
			}
			break;
		case ftp:
			break;
	}

	off_t temp_total_recv_bytes = total_received_bytes;
	while(total_received_bytes < file_length){
		usleep(1000000);
		if(total_received_bytes != temp_total_recv_bytes){
			progress = ((float)total_received_bytes/(float)file_length)*100;
			speed = (total_received_bytes-temp_total_recv_bytes); // received bytes per second
			cout<<"Proggress= "<<progress<<"% Speed = "<<speed/1024.0<<"KB/s"<<endl;
			temp_total_recv_bytes = total_received_bytes;
			//call_manager_stat_change((void*) &ptr_to_manager, Manager::status_changed);
		}
	}
	wait();
}
void Node::wrapper_to_get_status(void* ptr_to_object, int downloader_trd_index, off_t received_bytes, int stat_flag)
{
	static mutex mtx;
	mtx.lock();
	Node* my_self = (Node*)ptr_to_object;
	my_self->get_status(downloader_trd_index, received_bytes, stat_flag);
	mtx.unlock();
}
void Node::get_status(int downloader_trd_index, off_t received_bytes, int stat_flag)
{
	download_threads_it = download_threads.find(downloader_trd_index);
	if(download_threads_it!=download_threads.end()){
		total_received_bytes += received_bytes;
	}
}
