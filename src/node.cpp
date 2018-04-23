#include "node.h"
#include <unistd.h>
#include <cstdio>
#include <regex>
#include "manager.h"
#include "http_downloader.h"
#include "https_downloader.h"
Node::Node(addr_struct dwl_str_, int number_of_trds, void *pointer_to_manager):
	dwl_str(dwl_str_), num_of_trds(number_of_trds), ptr_to_manager(pointer_to_manager)

{
}
void Node::wait()
{
	for (map<int, Downloader*>::iterator it=download_threads.begin(); it!=download_threads.end(); ++it){
		it->second->join();
	}
}
void Node::run()
{
	Downloader* dwl;
	if (dwl_str.port == 80)
		dwl = new HttpDownloader(node_data, dwl_str, 0, 0, 0);
	else if(dwl_str.port == 443)
		dwl = new HttpsDownloader(node_data, dwl_str, 0, 0, 0);
	file_length = dwl->get_size();
	delete dwl;

	off_t trd_norm_len = file_length/num_of_trds;
	
	// Create file with specified size
	FILE* tmp_fp = fopen(dwl_str.file_name_on_server.c_str(), "r");
	if(!tmp_fp){
		FILE *fp = fopen(dwl_str.file_name_on_server.c_str(), "w");
		fseek(fp, file_length-1 , SEEK_SET);
		fputc('\0', fp);
		fclose(fp);
	}
	else
		fclose(tmp_fp);

	node_data = new node_struct;
	node_data->fp		= fopen(dwl_str.file_name_on_server.c_str(), "r+b");
	node_data->log_fp 	= fopen(("."+dwl_str.file_name_on_server+".LOG").c_str(), "r+");
	node_data->file_mutex	= new mutex;
	node_data->node 	= this;
	stopped_positions[0] = 0;
	if(node_data->log_fp){
		read_resume_log();
		node_data->resuming = true;
		for (map<int, int>::iterator it=start_positions.begin(); it!=start_positions.end(); ++it){
			if(it->first < start_positions.size()-1){
				trds_length[it->first] = start_positions[it->first+1]-stopped_positions[it->first];
			}
			else
				trds_length[it->first] = file_length - stopped_positions[it->first];
		}
	}
	else{
		node_data->resuming = false;
		node_data->log_fp = fopen(("."+dwl_str.file_name_on_server+".LOG").c_str(), "w");
		for(int i=0; i<num_of_trds; i++){
			off_t len = trd_norm_len;
			off_t pos = i*trd_norm_len;
			if(i == (num_of_trds-1)){
				len = file_length-(trd_norm_len*i);
				pos = i*trd_norm_len;
			}
			stopped_positions[i] = pos;
			trds_length[i] = len-1;
		}
	}

	Downloader *dnwl;
	switch(dwl_str.protocol){
		case kHttp:
			if( dwl_str.encrypted ){	
				for (map<int, int>::iterator it=stopped_positions.begin(); it!=stopped_positions.end(); ++it){
					dnwl = new HttpsDownloader(node_data, dwl_str,it->second,trds_length[it->first],it->first);
					dnwl->start();
					download_threads[it->first] = dnwl;
				}
			}
			else{
				for (map<int, int>::iterator it=stopped_positions.begin(); it!=stopped_positions.end(); ++it){
					dnwl = new HttpDownloader(node_data, dwl_str,it->second,trds_length[it->first],it->first);
					dnwl->start();
					download_threads[it->first] = dnwl;
				}
			}
			break;
		case kFtp:
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
	fclose(node_data->fp);
	fclose(node_data->log_fp);
	delete node_data;

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

bool Node::read_resume_log()
{
	fseek(node_data->log_fp, 0, SEEK_END);
	off_t fsize = ftell(node_data->log_fp);
	rewind(node_data->log_fp);
	char *buf = (char*)malloc(fsize + 1);
	fread(buf, fsize, 1, node_data->log_fp);
	buf[fsize] = 0;
	string buffer = string(buf);
	free(buf);
	node_data->log_buffer_str = buffer;

	regex *r = new regex("(p)(\\d+\t)(\\d+\n)");
	for(sregex_iterator i = sregex_iterator(buffer.begin(), buffer.end(), *r);i != sregex_iterator();++i ){
		smatch m = *i;
		stopped_positions[stoi(m[2])] = stoi(m[3]);
	}
	delete r;
	r = new regex("(s)(\\d+\t)(\\d+\n)");
	for(sregex_iterator i = sregex_iterator(buffer.begin(), buffer.end(), *r);i != sregex_iterator();++i ){
		smatch m = *i;
		start_positions[stoi(m[2])] = stoi(m[3]);
	}
	delete r;

	//p stopped_position
	//s start position for resume
	for (map<int, int>::iterator it=stopped_positions.begin(); it!=stopped_positions.end(); ++it){
		total_received_bytes += (it->second - start_positions[it->first]);
	}
}
