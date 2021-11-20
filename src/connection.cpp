#include "connection.h"

using namespace std;

ConnectionManager::ConnectionManager(shared_ptr<StateManager> state_manager)
  : state_manager(state_manager)
{
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

vector<uint16_t> ConnectionManager::get_indices_list()
{
  if (connections.size() == 0)
    init();
  vector<uint16_t> result;
  result.resize(connections.size());
  size_t vector_index = 0;
  for (const auto& connection : connections)
      result[vector_index++] = connection.first;
  return result;
}

SocketOps* ConnectionManager::get_sock_ops(uint16_t index) const
{
  return connections.at(index).socket_ops.get();
}

void ConnectionManager::set_sock_ops(unique_ptr<SocketOps> socket_ops,
                                     uint16_t index)
{
  connections.at(index).socket_ops = move(socket_ops);
}

ssize_t ConnectionManager::get_end_pos(uint16_t index) const
{
  return state_manager->get_end_pos(index);
}

ssize_t ConnectionManager::get_start_pos(uint16_t index) const
{
  return state_manager->get_start_pos(index);
}

ssize_t ConnectionManager::get_current_pos(uint16_t index) const
{
  return state_manager->get_current_pos(index);
}

bool ConnectionManager::get_init_stat(uint16_t index) const
{
  return connections.at(index).inited;
}

void ConnectionManager:: set_error(uint16_t index)
{
  set_init_stat(false, index);
  set_sock_ops(nullptr, index);
}

void ConnectionManager::set_init_stat(bool init_stat, uint16_t index)
{
  connections.at(index).inited = init_stat;
}

void ConnectionManager::survey_connections()
{
  // Remove finished connections
  vector<uint16_t> finished_connections;
  for (auto& [index, connection] : connections) {
    if (connection.inited == false)
      continue;
    const int64_t rem_len = state_manager->get_end_pos(index) -
                            state_manager->get_current_pos(index);
    if (rem_len <= 0)
      finished_connections.push_back(index);
  }
  for (size_t index : finished_connections) {
    state_manager->erase_part(index);
    connections.erase(index);
  }
}

void ConnectionManager::generate_one_connection(uint16_t index)
{
  connections[index] = Connection();
}

