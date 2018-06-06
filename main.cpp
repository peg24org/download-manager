#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "url_info.h"
#include "manager.h"
#include "definitions.h"
#include <unistd.h>
const char* program_name;
void print_usage (int exit_code)
{
	cerr << "Usage: "<< program_name << " options [ URL ]"<<endl;
	cerr <<" -h --help \n \
			Display this usage information.\n \
			-n number of connections\n";
	exit (exit_code);
}
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
	Manager mng;
	mng.new_download(dl_str,number_of_connections);
	mng.wait();
	return 0;
}

