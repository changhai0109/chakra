#ifndef DATABASE_ET_FEEDER
#define DATABASE_ET_FEEDER

#include <memory>
#include <string>
#include "et_feeder/et_feeder_node.h"
#include "third_party/db_as_map/db_as_map.hh"
#include "third_party/utils/protoio.hh"

typedef uint64_t NodeId;

namespace Chakra {

class DatabaseETFeeder {
 public:
  DatabaseETFeeder(std::string filename);
  ~DatabaseETFeeder();

  void addNode(std::shared_ptr<ETFeederNode> node);
  void removeNode(NodeId node_id);
  bool hasNodesToIssue() const;
  std::shared_ptr<ETFeederNode> getNextIssuableNode();
  void pushBackIssuableNode(NodeId node_id);
  std::shared_ptr<ETFeederNode> lookupNode(NodeId node_id);
  void freeChildrenNodes(NodeId parent_node_id);

 private:
  void readGlobalMetadata();
  void readProtoNodes();
  void readProtoNode();
  void readProtoNode(const ChakraProtoMsg::Node& node);
  void updateFreeNodes();

  ProtoInputStream trace;
  std::shared_ptr<ChakraProtoMsg::GlobalMetadata> globalMetaData;
  db_as_map::unordered_map<NodeId, ChakraProtoMsg::Node> chakraNodes;
  std::unordered_map<NodeId, std::unordered_set<NodeId>> parentMapChild;
  std::unordered_map<NodeId, std::unordered_set<NodeId>> childMapParent;
  std::set<NodeId> freeNodes;
  std::unordered_set<NodeId> onTheFlyNodes;
  uint64_t sys_id;
  bool et_complete;
};

} // namespace Chakra

#endif
