#include "et_feeder/et_feeder.h"

using namespace std;
using namespace Chakra;

ETFeeder::ETFeeder(string filename)
    : trace_(filename), window_size_(4096 * 256), et_complete_(false) {
  if (!trace_.is_open()) { // Assuming a method to check if file is open
    throw std::runtime_error("Failed to open trace file: " + filename);
  }

  try {
    readGlobalMetadata();
    readNextWindow();
  } catch (const std::exception& e) {
    cerr << "Error in constructor: " << e.what() << endl;
    throw; // Rethrow the exception for caller to handle
  }
}

ETFeeder::~ETFeeder() {}

void ETFeeder::addNode(shared_ptr<ETFeederNode> node) {
  dep_graph_[node->getChakraNode()->id()] = node;
}

void ETFeeder::removeNode(uint64_t node_id) {
  dep_graph_.erase(node_id);

  if (!et_complete_ && (dep_free_node_queue_.size() < window_size_)) {
    readNextWindow();
  }
}

bool ETFeeder::hasNodesToIssue() {
  return !(dep_graph_.empty() && dep_free_node_queue_.empty());
}

shared_ptr<ETFeederNode> ETFeeder::getNextIssuableNode() {
  if (dep_free_node_queue_.size() != 0) {
    shared_ptr<ETFeederNode> node = dep_free_node_queue_.top();
    dep_free_node_id_set_.erase(node->getChakraNode()->id());
    dep_free_node_queue_.pop();
    return node;
  } else {
    return nullptr;
  }
}

void ETFeeder::pushBackIssuableNode(uint64_t node_id) {
  shared_ptr<ETFeederNode> node = dep_graph_[node_id];
  dep_free_node_id_set_.emplace(node_id);
  dep_free_node_queue_.emplace(node);
}

shared_ptr<ETFeederNode> ETFeeder::lookupNode(uint64_t node_id) {
  return dep_graph_[node_id];
}

void ETFeeder::freeChildrenNodes(uint64_t node_id) {
  shared_ptr<ETFeederNode> node = dep_graph_[node_id];
  for (auto child : node->getChildren()) {
    auto child_chakra = child->getChakraNode();
    child->releaseParent(node_id);
    if (child->getUnreleasedParents().size() == 0) {
      dep_free_node_id_set_.emplace(child_chakra->id());
      dep_free_node_queue_.emplace(child);
    }
  }
}

void ETFeeder::readGlobalMetadata() {
  if (!trace_.is_open()) {
    throw runtime_error(
        "Trace file closed unexpectedly during reading global metadata.");
  }
  shared_ptr<ChakraProtoMsg::GlobalMetadata> pkt_msg =
      make_shared<ChakraProtoMsg::GlobalMetadata>();
  trace_.read(*pkt_msg);
}

shared_ptr<ETFeederNode> ETFeeder::readNode() {
  shared_ptr<ChakraProtoMsg::Node> pkt_msg =
      make_shared<ChakraProtoMsg::Node>();
  if (!trace_.read(*pkt_msg)) {
    return nullptr;
  }
  shared_ptr<ETFeederNode> node = make_shared<ETFeederNode>(pkt_msg);

  bool dep_unresolved = false;
  for (uint64_t parent_node_id : node->all_deps()) {
    auto parent_node = dep_graph_.find(parent_node_id);
    if (parent_node != dep_graph_.end()) {
      parent_node->second->addChild(node);
    } else {
      dep_unresolved = true;
      node->addDepUnresolvedParentID(parent_node_id);
    }
  }

  if (dep_unresolved) {
    dep_unresolved_node_set_.emplace(node);
  }

  return node;
}

void ETFeeder::resolveDep() {
  for (auto it = dep_unresolved_node_set_.begin();
       it != dep_unresolved_node_set_.end();) {
    shared_ptr<ETFeederNode> node = *it;
    vector<uint64_t> dep_unresolved_parent_ids =
        node->getDepUnresolvedParentIDs();
    for (auto inner_it = dep_unresolved_parent_ids.begin();
         inner_it != dep_unresolved_parent_ids.end();) {
      auto parent_node = dep_graph_.find(*inner_it);
      if (parent_node != dep_graph_.end()) {
        parent_node->second->addChild(node);
        inner_it = dep_unresolved_parent_ids.erase(inner_it);
      } else {
        ++inner_it;
      }
    }
    if (dep_unresolved_parent_ids.size() == 0) {
      it = dep_unresolved_node_set_.erase(it);
    } else {
      node->setDepUnresolvedParentIDs(dep_unresolved_parent_ids);
      ++it;
    }
  }
}

void ETFeeder::readNextWindow() {
  if (!trace_.is_open()) {
    throw runtime_error(
        "Trace file closed unexpectedly during reading next window.");
  }
  uint32_t num_read = 0;
  do {
    if (this->et_complete_) {
      // graph read finished, but still nodes unresolved
      // which means the graph is broken
      assert(false);
    }
    for (uint32_t num_read = 0; num_read < this->window_size_; num_read++) {
      std::shared_ptr<ETFeederNode> new_node = readNode();
      if (new_node == nullptr) {
        et_complete_ = true;
        break;
      }
      addNode(new_node);
    }
    resolveDep();
  } while (dep_unresolved_node_set_.size() != 0);

  for (auto node_id_node : dep_graph_) {
    uint64_t node_id = node_id_node.first;
    shared_ptr<ETFeederNode> node = node_id_node.second;
    if ((dep_free_node_id_set_.count(node_id) == 0) &&
        (node->getUnreleasedParents().size() == 0)) {
      dep_free_node_id_set_.emplace(node_id);
      dep_free_node_queue_.emplace(node);
    }
  }
}
