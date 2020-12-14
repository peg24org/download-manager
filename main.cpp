#include <getopt.h>

#include <cmath>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "node.h"

using namespace std;

const char* program_name;

string get_friendly_size_notation(size_t size)
{
  stringstream friendly_size;

  friendly_size << setprecision(2);
  if (size > pow(2, 10) && size < pow(2, 20))    // KB
    friendly_size << fixed << static_cast<float>(size) / pow(2, 10) << " KB";
  else if (size > pow(2, 20) && size < pow(2, 30))    // MB
    friendly_size << fixed << static_cast<float>(size) / pow(2, 20) << " MB";
  else if (size > pow(2, 30) && size < pow(2, 40))    // GB
    friendly_size << fixed << static_cast<float>(size) / pow(2, 30) << " GB";
  else
    friendly_size << size<< " B";

  return friendly_size.str();
}

string get_friendly_speed_notation(size_t size)
{
  stringstream friendly_size;

  if (size > pow(2, 10) && size < pow(2, 20))    // KB
    friendly_size << setprecision(0) << fixed << static_cast<float>(size) / pow(2, 10) << " KB";
  else if (size > pow(2, 20) && size < pow(2, 30))    // MB
    friendly_size << setprecision(1) << fixed << static_cast<float>(size) / pow(2, 20) << " MB";
  else if (size > pow(2, 30) && size < pow(2, 40))    // GB
    friendly_size << setprecision(2) << fixed << static_cast<float>(size) / pow(2, 30) << " GB";
  else
    friendly_size << size<< " B";

  return friendly_size.str();
}

void print_usage(int exit_code)
{
  cerr << "Usage: "<< program_name << " options [ URL ]"<<endl;
  cerr << "\t-h --help         display this usage information." << endl
       << "\t-n                number of connections" << endl
       << "\t-o                output file name" << endl
       << "\t-p --proxy        proxy address" << endl
       << "\t-l --speed_limit  download speed limit" << endl
       << "\t-t --timeout      timeout interval" << endl;
  exit(exit_code);
}

class DownloadMngr : public Node
{
  using Node::Node;

  size_t file_size = 0;
  size_t last_recv_bytes = 0;

  void on_data_received(size_t received_bytes, size_t speed)
  {
    float progress = (static_cast<float>(received_bytes) /
                      static_cast<float>(file_size)) * 100;

    cout << "\r" <<
      "Progress: " << fixed << setw(6) << setprecision(2) << progress << "%";

    cout << " Speed: " << setw(10) << get_friendly_speed_notation(speed)
         << "/s";
    cout << "\tReceived: " << setw(10)
         << get_friendly_size_notation(received_bytes);

    cout << flush;
  }
  void on_get_file_info(size_t node_index, size_t file_size,
                        const string& file_name)
  {
    cout << "File size: " << get_friendly_size_notation(file_size) << endl;
    this->file_size = file_size;
  }
};

int main(int argc, char* argv[])
{
  short int number_of_connections{1};
  string link;
  string optional_path;
  long int timeout{0};
  size_t speed_limit{0};
  string proxy_url;

  //**************** get command line arguments ***************
  int next_option;
  const char* const short_options = "hvo:n:t:p:l:";
  const struct option long_options[] = {
    {"help",        0, nullptr, 'h'},
    {"output",      1, nullptr, 'o'},
    {"verbose",     0, nullptr, 'v'},
    {"timeout",     1, nullptr, 't'},
    {"speed_limit", 1, nullptr, 'l'},
    {"proxy",       1, nullptr, 'p'},  // host:ip
    {nullptr,       0, nullptr, 0}
  };
  program_name = argv[0];

  if (argc < 2)
    print_usage(1);

  do {
    next_option = getopt_long(argc, argv, short_options,long_options, nullptr);
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
      case 'l':
        speed_limit= stoi(optarg);
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
  node->set_speed_limit(speed_limit);
  node->start();
  node->join();

  cout << endl;
  return 0;
}
