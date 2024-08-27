#ifndef CHAKRA__ET_OPERATOR__ET_FEEDER_H
#define CHAKRA__ET_OPERATOR__ET_FEEDER_H

#include <memory>
#include <unordered_map>

#include "et_feeder_node.h"
#include "raw_et_operator.h"

using NodeId = uint64_t;

namespace Chakra {
class ETFeeder {
 public:
  ETFeeder(std::string filename);
  ~ETFeeder();

  void addNode(std::shared_ptr<ETFeederNode> node) = delete;
  void removeNode(uint64_t node_id);
  bool hasNodesToIssue();
  std::shared_ptr<ETFeederNode> getNextIssuableNode();
  void pushBackIssuableNode(uint64_t node_id);
  std::shared_ptr<ETFeederNode> lookupNode(uint64_t node_id);
  void freeChildrenNodes(uint64_t node_id);

 private:
  ETOperator et_operator_;
  std::unordered_map<NodeId, std::weak_ptr<ETFeederNode>> node_map_{};
};
} // namespace Chakra

#endif
