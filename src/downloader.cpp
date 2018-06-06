#include "downloader.h"

#include <cassert>
#include <regex>
#include <cstdlib>

#include "node.h"

const string Downloader::
	HTTP_HEADER = "(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)";

Downloader::Downloader(node_struct* node_data, const addr_struct addr_data,
		size_t pos, size_t trd_length, int index)
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

void Downloader::write_to_file(size_t pos, size_t len, char* buf)
{
	node_data->file_mutex->lock();
	assert(node_data->fp);
	fseek(node_data->fp, pos, SEEK_SET);
	fwrite(buf, 1,len, node_data->fp);
	write_log_file(pos);
	node_data->file_mutex->unlock();
}

size_t Downloader::get_trd_len()
{
	return trd_len;
}

void Downloader::write_log_file(size_t pos)
{	
	regex e("(.|\\s)*(p"+to_string(index)+"\t\\d+\n)(.|\\s)*");
	if(node_data->log_buffer_str.length()<1)
		node_data->log_buffer_str = "p" + to_string(index) + "\t" +
			to_string(pos) + "\n";
	else{
		if (regex_match(node_data->log_buffer_str, e)) {
			regex re("p"+to_string(index)+"\\t\\d+\\n");
			node_data->log_buffer_str = regex_replace(node_data->log_buffer_str,
					re, "p" + to_string(index) + "\t" + to_string(pos) + "\n");
		}
		else
			node_data->log_buffer_str += "p" + to_string(index) + "\t" +
				to_string(pos) + "\n";
	}

	rewind(node_data->log_fp);
	fwrite(node_data->log_buffer_str.c_str(), 1,
			node_data->log_buffer_str.length(), node_data->log_fp);
	if(!is_start_pos_written && !node_data->resuming){
		write_start_pos_log(pos);
		is_start_pos_written = true;
	}
}

void Downloader::write_start_pos_log(size_t start_pos)
{
	node_data->log_buffer_str += "s" + to_string(index) + "\t" +
		to_string(start_pos)+"\n";
	rewind(node_data->log_fp);
	fwrite(node_data->log_buffer_str.c_str(), 1,
			node_data->log_buffer_str.length(), node_data->log_fp);
}

bool Downloader::regex_search_string(const string& input,
		const string& pattern, string& output)
{
	smatch m;
	regex e(pattern);
	bool retval = regex_search(input, m, e);
	output = m[2];
	return retval;
}

void Downloader::call_node_status_changed(int recieved_bytes, int err_flag)
{
	void (*call_back_func_ptr)(void* node, int downloader_trd_index,
			size_t recieved_bytes, int stat_flag);
	call_back_func_ptr = Node::wrapper_to_get_status;
	call_back_func_ptr(node_data->node, index, recieved_bytes, 0);
}
