#include <cmath>

#include <sstream>
#include <iomanip>
#include <iostream>

#include <getopt.h>

#include "node.h"

using namespace std;

const char* program_name;

string get_friendly_size_notation(size_t size)
{
  stringstream friendly_size;

  friendly_size << setprecision(3);
  if (size > pow(2, 10) && size < pow(2, 20))    // KB
    friendly_size << static_cast<float>(size) / pow(2, 10) << " KB";
  else if (size > pow(2, 20) && size < pow(2, 30))    // MB
    friendly_size << static_cast<float>(size) / pow(2, 20) << " MB";
  else if (size > pow(2, 30) && size < pow(2, 40))    // GB
    friendly_size << static_cast<float>(size) / pow(2, 30) << " GB";
  else
    friendly_size << size<< " B";

  return friendly_size.str();
}

void print_usage(int exit_code)
{
  cerr << "Usage: "<< program_name << " options [ URL ]"<<endl;
  cerr << "\t-h --help    Display this usage information." << endl
       << "\t-n number of connections" << endl
       << "\t-o output file name" << endl;
  exit(exit_code);
}

class DownloadMngr : public Node
{
  using Node::Node;

  size_t file_size = 0;
  size_t last_recv_bytes = 0;

  void on_data_received(size_t received_bytes)
  {
    float progress = (static_cast<float>(received_bytes) /
                      static_cast<float>(file_size)) * 100;

    cout << "\r" <<
      "Progress: " << fixed << setw(6) << setprecision(2) << progress << "%";

    static const float time_interval = callback_refresh_interval / 1000.0;
    float speed = 0;
    if (received_bytes != last_recv_bytes)
      speed = (received_bytes - last_recv_bytes) / time_interval;
    last_recv_bytes = received_bytes;

    cout << " Speed: " << setw(8) << get_friendly_size_notation(speed) << "/s";

    if (progress >= 100)
      cout << endl;
    cout << flush;
  }

  void on_get_file_info(size_t node_index, size_t file_size,
                        const string& file_name)
  {
    cout << "File size: " << get_friendly_size_notation(file_size)
         << " Bytes" << endl;
    this->file_size = file_size;
  }
};

int main(int argc, char* argv[])
{
  short int number_of_connections{1};
  string link;
  string optional_path;
  long int timeout{0};
  string proxy_url;

  //**************** get command line arguments ***************
  int next_option;
  const char* const short_options = "hvo:n:t:p:";
  const struct option long_options[] = {
    {"help",    0, NULL, 'h'},
    {"output",  1, NULL, 'o'},
    {"verbose", 0, NULL, 'v'},
    {"timeout", 1, NULL, 't'},
    {"proxy",   1, NULL, 'p'},  // host:ip
    {NULL,      0, NULL, 0}
  };
  program_name = argv[0];

  if (argc < 2)
    print_usage(1);

  do {
    next_option = getopt_long(argc, argv, short_options,long_options, NULL);
    switch(next_option) {
      case 'h':
        print_usage(0);
      case 'n':
        number_of_connections = stoi(optarg);
        break;
      case 'o':
        optional_path = optarg;
        break;
      case 't':
        timeout = stoi(optarg);
        break;
      case 'p':
        proxy_url = optarg;
        break;
      case '?':
        print_usage(1);
      case -1:
        break;
      default:
        abort();
    }
  } while (next_option != -1);

  for (int i = optind; i < argc; ++i)
    link = string(argv[i]);

  //******************************************

  unique_ptr<DownloadMngr> node = nullptr;

  if (timeout)
    node = make_unique<DownloadMngr>(link, optional_path,
                                     number_of_connections, timeout);
  else
    node = make_unique<DownloadMngr>(link, optional_path,
                                     number_of_connections);

  if (!proxy_url.empty())
    node->set_proxy(proxy_url);
  node->start();
  node->join();

  cout << endl;
  return 0;
}
