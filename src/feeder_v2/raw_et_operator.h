#ifndef CHAKRA_RAW_ET_OPERATOR_H
#define CHAKRA_RAW_ET_OPERATOR_H

#include <atomic>
#include <fstream>
#include <istream>
#include <list>
#include <mutex>
#include <streambuf>
#include <unordered_map>

#include "et_def.pb.h"
#include "../../third_party/astra-sim/common/Cache.hh"
#include "../../third_party/astra-sim/common/ProtobufUtils.hh"

using NodeId = uint64_t;
using ChakraNode = ChakraProtoMsg::Node;

namespace Chakra {
class _DependancyResolver {
 public:
  _DependancyResolver(bool enable_data_deps, bool enable_ctrl_deps)
      : enable_data_deps(enable_data_deps), enable_ctrl_deps(enable_ctrl_deps) {
    if (!enable_data_deps)
      if (!enable_ctrl_deps)
        throw std::runtime_error(
            "Should not create a dependancy resolver that resolves nothing");
  }

  _DependancyResolver() : _DependancyResolver(true, false) {}

  ~_DependancyResolver() {
    this->dependancy_map.clear();
    this->reverse_dependancy_map.clear();
    this->dependancy_free_nodes.clear();
  }

  void add_node(const ChakraNode& value);
  const std::unordered_set<NodeId>& get_dependancy_free_nodes() const;
  void take_node(const NodeId node_id);
  void push_back_node(const NodeId node_id);
  void finish_node(const NodeId node_id);
  void find_dependancy_free_nodes();

  const std::unordered_set<NodeId>& get_children(const NodeId node_id) const;
  const std::unordered_set<NodeId>& get_parents(const NodeId node_id) const;

 private:
  bool enable_data_deps;
  bool enable_ctrl_deps;
  std::unordered_map<NodeId, std::unordered_set<NodeId>> dependancy_map;
  std::unordered_map<NodeId, std::unordered_set<NodeId>> reverse_dependancy_map;
  std::unordered_set<NodeId> dependancy_free_nodes;
  std::unordered_set<NodeId> ongoing_nodes;
  void allocate_bucket(const NodeId& node_id);
  static const std::unordered_set<NodeId> empty_set;
};

class ETOperator {
 public:
  class Iterator {
    // why create iterator?
    // because try to avoid value copies.
    // 1. We can use shared_ptr to keep only one replication of the value,
    //    however, it is hard for cache design.
    // 2. We can use reference to avoid value copies, but we need a owner to
    //    hold that value, and holding will keep it in memory, which is what we
    //    try to avoid.
    // 3. We can use node_id as a pointer to the value, but every user need to
    //    get access to the ETFeeder to retrive the value, which is not
    //    feasible.
    // 4. We can use iterator to keep the value, and the iterator can be passed
    //    around, and the value can be retrived by the iterator.
   public:
    Iterator(ETOperator& handler, NodeId node_id)
        : handler(handler), node_id(node_id) {}

    const std::shared_ptr<const ChakraNode> operator*() const {
      // we only return const value, so that it saves lots of work to maintain
      // data consistency between memory and disk.
      return this->handler._get_node(node_id);
    }

    ETOperator& get_handler() {
      return handler;
    }

   private:
    ETOperator& handler;
    const NodeId node_id;
  };
  ChakraProtoMsg::GlobalMetadata global_metadata;
  ETOperator(const std::string& filename) : ETOperator(filename, 256) {};
  ETOperator(const std::string& filename, size_t capacity)
      : f(filename, std::ios::binary | std::ios::in | std::ios::app),
        cache(capacity),
        dependancy_resolver(true, true),
        data_dependancy_resolver(true, false) {
    build_index_cache();
  };
  ~ETOperator() {
    f.close();
  };
  Iterator get_node(NodeId node_id);
  _DependancyResolver dependancy_resolver;
  _DependancyResolver data_dependancy_resolver;

 private:
  void build_index_cache();
  const std::shared_ptr<const ChakraNode> _get_node(NodeId node_id);

  std::fstream f;
  AstraSim::Utils::Cache<NodeId, ChakraNode> cache;
  std::unordered_map<NodeId, std::streampos> index_map;
};
} // namespace Chakra

#endif
