#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "url_info.h"
#include "manager.h"
#include "definitions.h"
#include <unistd.h>
#include <functional>

using namespace std;

const char* program_name;

void print_usage (int exit_code)
{
	cerr << "Usage: "<< program_name << " options [ URL ]"<<endl;
	cerr <<" -h --help \n \
			Display this usage information.\n \
			-n number of connections\n";
	exit (exit_code);
}

class DownloadManager : public Manager
{
	size_t file_length = 0;
	size_t total_recv_bytes = 0;

	void on_get_file_stat(size_t node_index, size_t file_size ,addr_struct* addr_data)
	{
		cout << "File size = " << file_size << " Bytes." << endl;
		file_length = file_size;
	}
	std::chrono::time_point<std::chrono::system_clock> last_sample = std::chrono::system_clock::now();
	void on_status_changed(int downloader_trd_index, size_t total_trd_len ,size_t received_bytes,
			addr_struct* addr_data)
	{
		if (received_bytes > 0)
		{
			float progress =((float)total_recv_bytes / (float)file_length) * 100;
			std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end - last_sample;
			cout << "Progress = " << progress << "% Speed = " <<
				received_bytes / (1024 * elapsed_seconds.count()) << "KB/s" << endl;
			last_sample = std::chrono::system_clock::now();
			total_recv_bytes += received_bytes;
		}
	}
};

int main (int argc, char* argv[])
{
	short int number_of_connections = 1;
	string link;

	//**************** get command line arguments ***************
	int next_option;
	const char* const short_options = "hvo:n:";
	const struct option long_options[] = {
		{"help",	0, NULL, 'h'},
		{"output",	1, NULL, 'o'},
		{"verbose", 	0, NULL, 'v'},
		{NULL,		0, NULL,  0 }
	};
	program_name = argv[0];
	if(argc <2 )
		print_usage(1);
	do {
		next_option = getopt_long (argc, argv, short_options,long_options, NULL);
		switch (next_option){
			case 'h':
				print_usage (0);
			case 'n':
				number_of_connections = stoi(optarg);
				break;
			case '?':
				print_usage (1);
			case -1:
				break;
			default:
				abort ();
		}
	}while (next_option != -1);
	for (int i = optind; i < argc; ++i){
		link = string(argv[i]);
	}
	//******************************************

	URLInfo u_info(link);
	addr_struct dl_str = u_info.get_download_info();

	DownloadManager download_manager;
	download_manager.new_download(dl_str,number_of_connections);
	download_manager.wait();

	return 0;
}

