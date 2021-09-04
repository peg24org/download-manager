#include "connection.h"

using namespace std;

ConnectionManager::ConnectionManager(shared_ptr<StateManager> state_manager)
  : state_manager(state_manager)
{
  cerr << "Constructed....." << endl;
  cerr << __FILE__ << ":" << __LINE__ << endl;
  cerr << "Stat file siaz:" << state_manager->get_file_size() << endl;
  cerr << __FILE__ << ":" << __LINE__ << endl;
}

void ConnectionManager::set_parts_max(uint16_t parts_max)
{
  state_manager->set_chunks_num(parts_max);
}

void ConnectionManager::init()
{
  parts_list = state_manager->get_parts();
  for (uint16_t i : parts_list)
      generate_one_connection(i);
}

vector<uint16_t> ConnectionManager::get_indices_list() const
{
  vector<uint16_t> result;
  result.resize(connections.size());
  size_t vector_index = 0;
  for (const auto& connection : connections)
      result[vector_index++] = connection.first;
  return result;
}

bool& ConnectionManager::get_header_skipped_stat(uint16_t index)
{
  return connections.at(index).header_skipped;
}

SocketOps* ConnectionManager::get_sock_ops(uint16_t index) const
{
  return connections.at(index).socket_ops.get();
}

void ConnectionManager::set_sock_ops(unique_ptr<SocketOps> socket_ops,
                                     size_t index)
{
  connections[index].socket_ops = move(socket_ops);
}

ssize_t ConnectionManager::get_end_pos(int16_t index) const
{
  return state_manager->get_end_pos(index);
}

ssize_t ConnectionManager::get_start_pos(int16_t index) const
{
  return state_manager->get_start_pos(index);
}

ssize_t ConnectionManager::get_current_pos(int16_t index) const
{
  return state_manager->get_current_pos(index);
}

void ConnectionManager::survey_connections()
{
  // Remove finished connections
  vector<size_t> finished_connections;
  for (auto& [index, connection] : connections) {
    const int64_t rem_len = state_manager->get_end_pos(index) -
                            state_manager->get_current_pos(index);
    if ( rem_len <= 0)
      finished_connections.push_back(index);
  }
  for (size_t index : finished_connections)
    connections.erase(index);
}

void ConnectionManager::generate_one_connection(uint16_t index)
{
  //pair<size_t, Chunk> part = state_manager->get_part();
  // const size_t start = state_manager->get_start_pos(index);
  connections[index] = Connection();
}
