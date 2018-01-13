#include "downloader.h"
#include "definitions.h"
#include <cassert>
Downloader::Downloader()
{
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
void Downloader::set_node_mutex_and_fd(FILE *fd, mutex* file_mutex)
{
	this->fd = fd;
	this->file_mutex = file_mutex;
}
void Downloader::run() 
{
	downloader_trd();
}
void Downloader::write_to_file(off_t pos, off_t len, char* buf)
{
	file_mutex->lock();
	assert(fd);
	fseek( fd, pos, SEEK_SET );
	fwrite(buf, 1, len, fd);
	file_mutex->unlock();
}
off_t Downloader::get_trd_len()
{
	return trd_len;
}
