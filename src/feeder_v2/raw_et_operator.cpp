#include "raw_et_operator.h"

using namespace Chakra;

void ETOperator::build_index_cache() {
  this->f.clear();
  this->f.seekg(0, std::ios::beg);
  ChakraProtoMsg::Node node;
  bool ret;
  ret = _ProtobufUtils::readMessage(f, global_metadata);
  assert(ret);
  std::streampos last_pos = f.tellg();
  while (true) {
    ret = _ProtobufUtils::readMessage(f, node);
    if (!ret)
      break;
    const auto& node_id = node.id();
    this->cache.put(node_id, node);
    index_map[node_id] = last_pos;
    last_pos = f.tellg();
    this->dependancy_resolver.add_node(node);
  }
  this->dependancy_resolver.find_dependancy_free_nodes();
  this->f.clear();
  this->f.seekg(0, std::ios::beg);
}

const ChakraNode& ETOperator::_get_node(NodeId node_id) {
  // Why return const value:
  // for easy data consistency management between memory and disk.
  if (this->cache.has(node_id))
    return this->cache.get(node_id);
  if (this->index_map.find(node_id) == this->index_map.end()) {
    throw std::runtime_error("Node not found in index");
  }
  this->f.clear();
  this->f.seekg(this->index_map[node_id], std::ios::beg);
  ChakraNode node;
  _ProtobufUtils::readMessage(f, node);
  this->cache.put(node_id, node);
  return this->cache.get(node_id);
}

ETOperator::Iterator ETOperator::get_node(NodeId node_id) {
  return Iterator(*this, node_id);
}

void _DependancyResolver::add_node(const ChakraNode& node) {
  const auto node_id = node.id();
  this->allocate_bucket(node_id);
  if (this->enable_data_deps)
    for (const auto data_dep : node.data_deps()) {
      this->allocate_bucket(data_dep);
      this->dependancy_map[node_id].insert(data_dep);
      this->reverse_dependancy_map[data_dep].insert(node_id);
    }
  if (this->enable_ctrl_deps)
    for (const auto ctrl_dep : node.ctrl_deps()) {
      this->allocate_bucket(ctrl_dep);
      this->dependancy_map[node_id].insert(ctrl_dep);
      this->reverse_dependancy_map[ctrl_dep].insert(node_id);
    }
}

void _DependancyResolver::take_node(const NodeId node_id) {
  if (this->dependancy_map.find(node_id) == this->dependancy_map.end())
    throw std::runtime_error("Node not found in dependancy map");
  if (this->dependancy_free_nodes.find(node_id) ==
      this->dependancy_free_nodes.end())
    throw std::runtime_error("Node not dependancy free, taken, or finished");
  this->ongoing_nodes.insert(node_id);
  this->dependancy_free_nodes.erase(node_id);
}

void _DependancyResolver::push_back_node(const NodeId node_id) {
  if (this->dependancy_map.find(node_id) == this->dependancy_map.end())
    throw std::runtime_error("Node not found in dependancy map");
  if (this->ongoing_nodes.find(node_id) == this->ongoing_nodes.end())
    throw std::runtime_error("Node not found in ongoing nodes");
  this->dependancy_free_nodes.insert(node_id);
  this->ongoing_nodes.erase(node_id);
}

void _DependancyResolver::finish_node(const NodeId node_id) {
  if (this->dependancy_map.find(node_id) == this->dependancy_map.end())
    throw std::runtime_error("Node not found in dependancy map");
  if (this->ongoing_nodes.find(node_id) == this->ongoing_nodes.end())
    throw std::runtime_error("Node not found in ongoing nodes");

  this->ongoing_nodes.erase(node_id);

  for (const auto& dependant : this->reverse_dependancy_map[node_id]) {
    this->dependancy_map[dependant].erase(node_id);
    if (this->dependancy_map[dependant].empty())
      this->dependancy_free_nodes.insert(dependant);
  }
  this->dependancy_map.erase(node_id);
  this->reverse_dependancy_map.erase(node_id);
}

const std::unordered_set<NodeId>& _DependancyResolver::
    get_dependancy_free_nodes() const {
  return this->dependancy_free_nodes;
}

void _DependancyResolver::find_dependancy_free_nodes() {
  for (const auto& entry : this->dependancy_map)
    if (entry.second.empty())
      this->dependancy_free_nodes.insert(entry.first);
}

void _DependancyResolver::allocate_bucket(const NodeId& node_id) {
  if (this->dependancy_map.find(node_id) == this->dependancy_map.end())
    this->dependancy_map[node_id] = std::unordered_set<NodeId>();
  if (this->reverse_dependancy_map.find(node_id) ==
      this->reverse_dependancy_map.end())
    this->reverse_dependancy_map[node_id] = std::unordered_set<NodeId>();
}

bool _ProtobufUtils::readVarint32(std::istream& f, uint32_t& value) {
  uint8_t byte;
  value = 0;
  int8_t shift = 0;
  while (f.read(reinterpret_cast<char*>(&byte), 1)) {
    value |= (byte & 0x7f) << shift;
    if (!(byte & 0x80))
      return true;
    shift += 7;
    if (shift > 28)
      return false;
  }
  return false;
}

template <typename T>
bool _ProtobufUtils::readMessage(std::istream& f, T& msg) {
  if (f.eof())
    return false;
  static char buffer[8192];
  uint32_t size;
  if (!readVarint32(f, size))
    return false;
  if (size > 8192)
    return false;
  f.read(buffer, size);
  buffer[size] = 0;
  msg.ParseFromArray(buffer, size);
  return true;
}

template <typename K, typename T>
void _Cache<K, T>::put(K id, const T& node) {
  if (this->cache.find(id) != this->cache.end()) {
    this->lru.erase(cache[id].second);
    this->lru.push_back(id);
    this->cache[id].second = --this->lru.end();
    this->cache[id].first = node;
  } else {
    if (cache.size() >= capacity) {
      auto lru_node = this->lru.front();
      this->cache.erase(lru_node);
      this->lru.pop_front();
    }
    this->lru.push_back(id);
    this->cache[id] = {node, --this->lru.end()};
  }
}

template <typename K, typename T>
bool _Cache<K, T>::has(K id) const {
  return this->cache.find(id) != this->cache.end();
}

template <typename K, typename T>
const T& _Cache<K, T>::get(K id) {
  if (!this->has(id))
    throw std::runtime_error("Node not found in cache");
  return this->cache[id].first;
}

template <typename K, typename T>
void _Cache<K, T>::remove(K id) {
  if (!this->has(id)) {
    return;
  }
  this->lru.erase(cache[id].second);
  this->cache.erase(id);
}
