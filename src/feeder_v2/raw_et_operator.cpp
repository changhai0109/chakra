#include "raw_et_operator.h"

#include <atomic>
#include <cassert>
#include <fstream>
#include <istream>
#include <list>
#include <mutex>
#include <streambuf>
#include <thread>
#include <unordered_map>
#include "../../third_party/astra-sim/common/Cache.hh"
#include "../../third_party/astra-sim/common/ProtobufUtils.hh"

using namespace Chakra;
using namespace AstraSim::Utils;

void ETOperator::build_index_cache() {
  this->f.clear();
  this->f.seekg(0, std::ios::beg);
  ChakraProtoMsg::Node node;
  bool ret;
  ret = ProtobufUtils::readMessage(f, global_metadata);
  assert(ret);
  std::streampos last_pos = f.tellg();
  while (true) {
    ret = ProtobufUtils::readMessage(f, node);
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

const std::shared_ptr<const ChakraNode> ETOperator::_get_node(NodeId node_id) {
  // Why return const value:
  // for easy data consistency management between memory and disk.
  try {
    return this->cache.get_locked(node_id);
  } catch (const std::runtime_error& e) {
    // cache miss
  }
  if (this->index_map.find(node_id) == this->index_map.end()) {
    throw std::runtime_error("Node not found in index");
  }
  this->f.clear();
  this->f.seekg(this->index_map[node_id], std::ios::beg);
  ChakraNode node;
  ProtobufUtils::readMessage(f, node);
  this->cache.put(node_id, node);
  return this->cache.get_locked(node_id);
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

  for (const NodeId dependant : this->reverse_dependancy_map[node_id]) {
    // TODO: iterate during change for reverse_dependancy_map, fix it.
    this->dependancy_map[dependant].erase(node_id);
    if (this->dependancy_map[dependant].empty()) {
      this->dependancy_free_nodes.insert(dependant);
      // this->dependancy_map.erase(dependant);
    }
  }
  this->reverse_dependancy_map[node_id].clear();
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

const std::unordered_set<NodeId> _DependancyResolver::empty_set;

const std::unordered_set<NodeId>& _DependancyResolver::get_children(
    const NodeId node_id) const {
  if (this->dependancy_map.find(node_id) == this->dependancy_map.end())
    return _DependancyResolver::empty_set;
  return this->dependancy_map.at(node_id);
}

const std::unordered_set<NodeId>& _DependancyResolver::get_parents(
    const NodeId node_id) const {
  if (this->reverse_dependancy_map.find(node_id) ==
      this->reverse_dependancy_map.end())
    return _DependancyResolver::empty_set;
  return this->reverse_dependancy_map.at(node_id);
}

void _DependancyResolver::allocate_bucket(const NodeId& node_id) {
  if (this->dependancy_map.find(node_id) == this->dependancy_map.end())
    this->dependancy_map[node_id] = std::unordered_set<NodeId>();
  if (this->reverse_dependancy_map.find(node_id) ==
      this->reverse_dependancy_map.end())
    this->reverse_dependancy_map[node_id] = std::unordered_set<NodeId>();
}
