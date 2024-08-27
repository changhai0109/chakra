#ifndef CHAKRA_RAW_ET_OPERATOR_H
#define CHAKRA_RAW_ET_OPERATOR_H

#include <fstream>
#include <istream>
#include <list>
#include <streambuf>
#include <unordered_map>

#include "et_def.pb.h"

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

  void add_node(const ChakraNode& node);
  const std::unordered_set<NodeId>& get_dependancy_free_nodes() const;
  void take_node(const NodeId node_id);
  void push_back_node(const NodeId node_id);
  void finish_node(const NodeId node_id);
  void find_dependancy_free_nodes();

 private:
  bool enable_data_deps;
  bool enable_ctrl_deps;
  std::unordered_map<NodeId, std::unordered_set<NodeId>> dependancy_map;
  std::unordered_map<NodeId, std::unordered_set<NodeId>> reverse_dependancy_map;
  std::unordered_set<NodeId> dependancy_free_nodes;
  std::unordered_set<NodeId> ongoing_nodes;
  void allocate_bucket(const NodeId& node_id);
};

class _ProtobufUtils {
 public:
  static bool readVarint32(std::istream& f, uint32_t& value);

  template <typename T>
  static bool readMessage(std::istream& f, T& msg);
};

template <typename K, typename T>
class _Cache {
 public:
  _Cache(size_t capacity) : capacity(capacity) {}

  void put(K id, const T& node);

  bool has(K id) const;

  const T& get(K id);

  void remove(K id);

  ~_Cache() {
    cache.clear();
    lru.clear();
  }

 private:
  size_t capacity;
  std::unordered_map<K, std::pair<T, typename std::list<K>::iterator>> cache;
  std::list<K> lru;
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

    const ChakraNode operator*() const {
      // we only return const value, so that it saves lots of work to maintain
      // data consistency between memory and disk.
      return this->get_value();
    }

    const ChakraNode get_value() const {
      // for people who want to get the value of the node
      // use this function
      return handler._get_node(node_id);
    }

    const ChakraNode& get_ref() const {
      // for people who want to get the reference of the node to avoid value
      // copy use this function
      // caution: the reference might be invalid if the
      // node is removed from the cache, should only used for inplace operators.
      return handler._get_node(node_id);
    }

    const ETOperator& get_handler() const {
      return handler;
    }

   private:
    ETOperator& handler;
    NodeId node_id;
  };
  ChakraProtoMsg::GlobalMetadata global_metadata;
  ETOperator(const std::string& filename) : ETOperator(filename, 4096) {};
  ETOperator(const std::string& filename, size_t capacity)
      : f(filename, std::ios::binary | std::ios::in | std::ios::app),
        cache(capacity) {
    build_index_cache();
  };
  ~ETOperator() {
    f.close();
  };
  Iterator get_node(NodeId node_id);
  _DependancyResolver dependancy_resolver;

 private:
  void build_index_cache();
  const ChakraNode& _get_node(NodeId node_id);

  _Cache<NodeId, ChakraNode> cache;
  std::unordered_map<NodeId, std::streampos> index_map;
  std::fstream f;
};
} // namespace Chakra

#endif
