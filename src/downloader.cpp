#include "downloader.h"
#include "definitions.h"
#include <cassert>
#include <regex>
#include <cstdlib>
Downloader::Downloader(node_struct* node_data, const addr_struct addr_data, off_t pos, off_t trd_length, int index)
{
	this->node_data	= node_data;
	this->addr_data	= addr_data;
	this->pos	= pos;
	this->trd_len	= trd_length;
	this->index	= index;
}
Downloader::~Downloader()
{
	try {
		if(threadObject)
			delete threadObject;
	}
	catch(...){
		std::cerr<<"Pointer delete exception."<<std::endl;
	}
}
int Downloader::get_index()
{
	return index;
}
void Downloader::set_index(int value)
{
	index = value;
}
void Downloader::run() 
{
	downloader_trd();
}
void Downloader::write_to_file(off_t pos, off_t len, char* buf)
{
	node_data->file_mutex->lock();
	assert(node_data->fp);
	fseek(node_data->fp, pos, SEEK_SET);
	fwrite(buf, 1,len, node_data->fp);
	write_log_file(pos);
	node_data->file_mutex->unlock();
}
off_t Downloader::get_trd_len()
{
	return trd_len;
}
void Downloader::write_log_file(off_t pos)
{	
	regex e ("(.|\\s)*(p"+to_string(index)+"\t\\d+\n)(.|\\s)*");
	if(node_data->log_buffer_str.length()<1){
		node_data->log_buffer_str = "p"+to_string(index)+"\t"+to_string(pos)+"\n";
	}
	else{

		if(regex_match(node_data->log_buffer_str, e)){
			regex re("p"+to_string(index)+"\\t\\d+\\n");

			node_data->log_buffer_str = regex_replace(node_data->log_buffer_str,re,"p"+to_string(index)+"\t"+to_string(pos)+"\n");
		}
		else{
			node_data->log_buffer_str += "p"+to_string(index)+"\t"+to_string(pos)+"\n";
		}
	}


	rewind(node_data->log_fp);
	fwrite(node_data->log_buffer_str.c_str(), 1, node_data->log_buffer_str.length(), node_data->log_fp);
	if(!is_start_pos_written && !node_data->resuming){
		write_start_pos_log(pos);
		is_start_pos_written = true;
	}
}
void Downloader::write_start_pos_log(off_t start_pos)
{
	node_data->log_buffer_str += "s"+to_string(index)+"\t"+to_string(start_pos)+"\n";
	rewind(node_data->log_fp);
	fwrite(node_data->log_buffer_str.c_str(), 1, node_data->log_buffer_str.length(), node_data->log_fp);
}
