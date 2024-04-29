#include "et_feeder/database_et_feeder.h"
#include <iostream>

using namespace Chakra;

DatabaseETFeeder::DatabaseETFeeder(std::string filename)
    : trace(filename), et_complete(false) {
  size_t startPos = filename.find(".");
  size_t endPos = filename.find_last_of(".");
  this->sys_id =
      std::stol(filename.substr(startPos + 1, endPos - startPos - 1));

  readGlobalMetadata();
  readProtoNodes();
}

DatabaseETFeeder::~DatabaseETFeeder() {}

void DatabaseETFeeder::readGlobalMetadata() {
  if (!trace.is_open()) {
    throw std::runtime_error(
        "Trace file closed unexpectedly during reading global metadata.");
  }
  this->globalMetaData = std::make_shared<ChakraProtoMsg::GlobalMetadata>();
  trace.read(*this->globalMetaData);
}

void DatabaseETFeeder::readProtoNode() {
  if (!trace.is_open()) {
    throw std::runtime_error(
        "Trace file closed unexpectedly during reading proto node.");
  }
  ChakraProtoMsg::Node node;
  if (!trace.read(node)) {
    et_complete = true;
    return;
  }
  this->readProtoNode(node);
}

void DatabaseETFeeder::readProtoNode(const ChakraProtoMsg::Node& node) {
  this->chakraNodes.insert(node.id(), node);

  this->childMapParent.emplace(node.id(), std::unordered_set<NodeId>());
  if (this->parentMapChild.find(node.id()) == this->parentMapChild.end())
    this->parentMapChild.emplace(node.id(), std::unordered_set<NodeId>());
  for (NodeId parent_node_id : node.data_deps()) {
    if (this->parentMapChild.find(parent_node_id) == this->parentMapChild.end())
      this->parentMapChild.emplace(
          parent_node_id, std::unordered_set<NodeId>());
    this->childMapParent[node.id()].insert(parent_node_id);
    this->parentMapChild[parent_node_id].insert(node.id());
  }
  for (NodeId parent_node_id : node.ctrl_deps()) {
    if (this->parentMapChild.find(parent_node_id) == this->parentMapChild.end())
      this->parentMapChild.emplace(
          parent_node_id, std::unordered_set<NodeId>());
    this->childMapParent[node.id()].insert(parent_node_id);
    this->parentMapChild[parent_node_id].insert(node.id());
  }
}

void DatabaseETFeeder::readProtoNodes() {
  while (trace.is_open() && (!et_complete)) {
    readProtoNode();
  }
  this->updateFreeNodes();
}

void DatabaseETFeeder::updateFreeNodes() {
  for (const auto& it : this->childMapParent) {
    if (it.second.size() == 0 && !this->onTheFlyNodes.count(it.first)) {
      this->freeNodes.insert(it.first);
    }
  }
}

void DatabaseETFeeder::addNode(std::shared_ptr<ETFeederNode> node) {
  ChakraProtoMsg::Node chakraNode = *(node->getChakraNode());
  this->readProtoNode(chakraNode);
  this->updateFreeNodes();
}

void DatabaseETFeeder::removeNode(NodeId node_id) {
  // std::cout << "remove node sys_id=" << this->sys_id << " node_id=" <<
  // node_id
  // << std::endl;
  // this->freeChildrenNodes(node_id);
  // this->chakraNodes.erase(node_id);
  // this->onTheFlyNodes.erase(node_id);
  // this->freeNodes.erase(node_id);
  // this->parentMapChild.erase(node_id);
  // this->childMapParent.erase(node_id);
}

bool DatabaseETFeeder::hasNodesToIssue() const {
  return !this->freeNodes.empty();
}

std::shared_ptr<ETFeederNode> DatabaseETFeeder::getNextIssuableNode() {
  if (!this->hasNodesToIssue()) {
    return nullptr;
  }
  NodeId node_id = *this->freeNodes.begin();
  this->freeNodes.erase(node_id);
  this->onTheFlyNodes.insert(node_id);
  return this->lookupNode(node_id);
}

void DatabaseETFeeder::pushBackIssuableNode(NodeId node_id) {
  this->onTheFlyNodes.erase(node_id);
  this->freeNodes.insert(node_id);
}

std::shared_ptr<ETFeederNode> DatabaseETFeeder::lookupNode(NodeId node_id) {
  std::shared_ptr<ChakraProtoMsg::Node> chakraNode =
      std::make_shared<ChakraProtoMsg::Node>(this->chakraNodes.at(node_id));
  return std::make_shared<ETFeederNode>(chakraNode);
}

void DatabaseETFeeder::freeChildrenNodes(NodeId parent_node_id) {
  for (NodeId child_node_id : this->parentMapChild.at(parent_node_id)) {
    this->childMapParent.at(child_node_id).erase(parent_node_id);
    if (this->childMapParent.at(child_node_id).empty()) {
      if (!this->onTheFlyNodes.count(child_node_id))
        this->freeNodes.insert(child_node_id);
    }
  }
}
static std::unique_ptr<void, decltype(&std::free)> buffer(nullptr, &std::free);
static size_t bufferSize = 0ul;

template <>
struct db_as_map::Serialize<ChakraProtoMsg::Node> {
  static void* serialize(const ChakraProtoMsg::Node& value) {
    size_t size = value.ByteSizeLong();
    if (size > bufferSize) {
      buffer.reset(std::malloc(size));
      bufferSize = size;
    }
    if (!value.SerializeToArray(buffer.get(), size))
      throw std::runtime_error("Failed to serialize ChakraProtoMsg::Node");
    return buffer.get();
  }
  static size_t size(const ChakraProtoMsg::Node& value) {
    return value.ByteSizeLong();
  }
  static ChakraProtoMsg::Node deserialize(const void* value, size_t size) {
    ChakraProtoMsg::Node node;
    if (!node.ParseFromArray(value, size))
      throw std::runtime_error("Failed to deserialize ChakraProtoMsg::Node");
    return node;
  }
};
