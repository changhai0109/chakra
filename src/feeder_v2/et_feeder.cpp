#include "et_feeder.h"

using namespace Chakra;

ETFeeder::ETFeeder(std::string filename) : et_operator_(filename) {}

ETFeeder::~ETFeeder() = default;

void ETFeeder::removeNode(uint64_t node_id) {
  // do nothing
}

bool ETFeeder::hasNodesToIssue() {
  return this->et_operator_.dependancy_resolver.get_dependancy_free_nodes()
             .size() > 0;
}

std::shared_ptr<ETFeederNode> ETFeeder::getNextIssuableNode() {
  const auto& node_id_iter =
      this->et_operator_.dependancy_resolver.get_dependancy_free_nodes()
          .begin();
  if (node_id_iter ==
      this->et_operator_.dependancy_resolver.get_dependancy_free_nodes()
          .end()) {
    return nullptr;
  }
  const auto node_id = *node_id_iter;
  // const auto& node_id =
  // *this->et_operator_.dependancy_resolver.get_dependancy_free_nodes()
  //  .begin();
  this->et_operator_.dependancy_resolver.take_node(node_id);
  std::shared_ptr<ETFeederNode> node =
      std::make_shared<ETFeederNode>(this->et_operator_.get_node(node_id));
  this->node_map_[node_id] = node;
  return node;
}

void ETFeeder::pushBackIssuableNode(uint64_t node_id) {
  this->et_operator_.dependancy_resolver.push_back_node(node_id);
}

std::shared_ptr<ETFeederNode> ETFeeder::lookupNode(uint64_t node_id) {
  std::shared_ptr<ETFeederNode> node; // keep it alive until it is returned
  if (this->node_map_.find(node_id) == this->node_map_.end() ||
      this->node_map_[node_id].expired()) {
    node = std::make_shared<ETFeederNode>(this->et_operator_.get_node(node_id));
    this->node_map_[node_id] = node;
  }
  return this->node_map_[node_id].lock();
}

void ETFeeder::freeChildrenNodes(uint64_t node_id) {
  this->et_operator_.dependancy_resolver.finish_node(node_id);
}
