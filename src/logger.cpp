#include "logger.h"

#include <iostream>
#include <bits/stdc++.h>

using namespace std;

Logger::Logger(const string& file_name)
  : file_size(0)
  , log_file(file_name)
{
}

void Logger::create(size_t file_size)
{
  log_file.create();
  this->file_size = file_size;
}

void Logger::open()
{
  /**
   * <file size> <number of threads>
   * <thread start byte> <downloaded end byte>
   * ...
   */
  log_file.open();
}

void Logger::write(int thread_index, size_t last_byte)
{
  threads_log[thread_index].second = last_byte;
  string log_data = to_string(file_size) + "\n";
  write_log_to_file();
}

void Logger::write_first_position(int thread_index, size_t first_position)
{
  threads_log[thread_index].first = first_position;
  string log_data = to_string(file_size) + "\n";
  write_log_to_file();
}

ThreadsLog Logger::get_threads_data()
{
  istringstream file_contents(log_file.get_file_contents());
  file_contents >> file_size;

  int thread_index;
  size_t start_byte, end_byte;
  while(file_contents >> thread_index >> start_byte >> end_byte) {
    cout << thread_index <<" " << start_byte << " " << end_byte << endl;
    threads_log[thread_index] = make_pair(start_byte, end_byte);
  }

  return threads_log;
}

void Logger::write_log_to_file()
{
  string log_data = to_string(file_size) + "\n";

  for (auto thread_log: threads_log) {
    // Thread index
    log_data += to_string(thread_log.first) + " ";
    // First byte of thread
    log_data += to_string(thread_log.second.first) + " ";
    // Last downloaded byte of thread
    log_data += to_string(thread_log.second.second) + "\n";
  }

  log_file.write(log_data.c_str(), log_data.length());
}

void Logger::remove()
{
  log_file.remove();
}
