#include <getopt.h>

#include <iomanip>
#include <iostream>

#include "node.h"
#include "url_info.h"

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

class DownloadMngr : public Node
{
  using Node::Node;

  size_t file_length = 0;
  size_t last_recv_bytes = 0;
  float speed = 0;

  void on_data_received(const std::map<int, DownloadChunk>& download_chunks)
  {
    size_t received_bytes = 0;
    for (auto chunk : download_chunks)
      received_bytes += chunk.second.current_pos - chunk.second.start_pos;

    float progress = (static_cast<float>(received_bytes) /
        static_cast<float>(file_length)) * 100;

    cout << "\r" <<
      "Progress: " << fixed << setw(6) << setprecision(2) << progress << "%";

    if(received_bytes != last_recv_bytes)
      speed = (1000 * (received_bytes - last_recv_bytes)) /
        (callback_refresh_interval * 1024);
    last_recv_bytes = received_bytes;
    string speed_unit = "KB";

    if (speed > 1024) {
      speed_unit = "MB";
      speed /= 1024;
    }
    cout << " Speed: " << setw(6) << setprecision(2) << speed <<
      speed_unit + "/s" << flush;


    if (progress >= 100)
      cout << endl;
    cout << flush;
  }

  void on_get_file_stat(size_t node_index, size_t file_size,
      struct addr_struct* addr_data)
  {
    cout << "File size: " << file_size << " Bytes" << endl;
    file_length = file_size;
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
    {"help",    0, NULL, 'h'},
    {"output",  1, NULL, 'o'},
    {"verbose", 0, NULL, 'v'},
    {NULL,      0, NULL, 0}
  };
  program_name = argv[0];

  if (argc < 2)
    print_usage(1);

  do {
    next_option = getopt_long (argc, argv, short_options,long_options, NULL);
    switch (next_option) {
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
  } while (next_option != -1);

  for (int i = optind; i < argc; ++i)
    link = string(argv[i]);

  //******************************************

  URLInfo u_info(link);
  struct addr_struct dl_str = u_info.get_download_info();

  DownloadMngr nd(dl_str, number_of_connections);
  nd.start();
  nd.join();

  cout << endl;

  return 0;
}
