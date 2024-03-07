#include "et_feeder/et_feeder_node.h"

using namespace std;
using namespace Chakra;

ETFeederNode::ETFeederNode(std::shared_ptr<ChakraProtoMsg::Node> node) {
  this->node_ = node;
  this->id_ = node->id();
  this->name_ = node->name();
  this->runtime_ = node->duration_micros();

  for (const auto& attr : node->attr()) {
    const string& attr_name = attr.name();
    this->attrs_.emplace(attr_name, attr);
  }
}

shared_ptr<ChakraProtoMsg::Node> ETFeederNode::getChakraNode() {
  return node_;
}

void ETFeederNode::addChild(shared_ptr<ETFeederNode> node) {
  // Avoid adding the same child node multiple times
  // addChild is called multiple times to resolve dependencies
  if (children_set_.find(node) != children_set_.end()) {
    return;
  }
  children_vec_.emplace_back(node);
  children_set_.emplace(node);
}

vector<shared_ptr<ETFeederNode>> ETFeederNode::getChildren() {
  return children_vec_;
}

void ETFeederNode::addDepUnresolvedParentID(uint64_t node_id) {
  dep_unresolved_parent_ids_.emplace_back(node_id);
}

vector<uint64_t> ETFeederNode::getDepUnresolvedParentIDs() {
  return dep_unresolved_parent_ids_;
}

void ETFeederNode::setDepUnresolvedParentIDs(
    vector<uint64_t> const& dep_unresolved_parent_ids) {
  dep_unresolved_parent_ids_ = dep_unresolved_parent_ids;
}

const ChakraProtoMsg::AttributeProto& ETFeederNode::get_attr(
    const string& attr_name) const {
  if (this->has_attr(attr_name))
    return this->attrs_.at(attr_name);
  throw std::runtime_error(
      "Asked for attr \"" + attr_name + "\" from node " +
      std::to_string(this->id_) + ", which do not exist");
}

bool ETFeederNode::has_attr(const string& attr_name) const {
  const auto& item = this->attrs_.find(attr_name);
  return item != this->attrs_.end();
}

uint64_t ETFeederNode::id() {
  return id_;
}

string ETFeederNode::name() {
  return name_;
}

bool ETFeederNode::is_cpu_op() {
  if (!this->has_attr("is_cpu_op"))
    return true;
  const auto& attr = this->get_attr("is_cpu_op");
  return static_cast<uint32_t>(attr.int32_val());
}

ChakraProtoMsg::NodeType ETFeederNode::type() {
  return node_->type();
}

uint64_t ETFeederNode::runtime() {
  return runtime_;
}

uint64_t ETFeederNode::num_ops() {
  const auto& attr = this->get_attr("num_ops");
  return static_cast<uint64_t>(attr.int64_val());
}

uint32_t ETFeederNode::tensor_loc() {
  const auto& attr = this->get_attr("tensor_loc");
  return static_cast<uint32_t>(attr.int32_val());
}

uint64_t ETFeederNode::tensor_size() {
  const auto& attr = this->get_attr("tensor_size");
  return static_cast<uint64_t>(attr.uint64_val());
}

ChakraProtoMsg::CollectiveCommType ETFeederNode::comm_type() {
  const auto& attr = this->get_attr("comm_type");
  return static_cast<ChakraProtoMsg::CollectiveCommType>(attr.int64_val());
}

uint32_t ETFeederNode::involved_dim_size() {
  const auto& attr = this->get_attr("involved_dim");
  return static_cast<uint32_t>(attr.bool_list().values().size());
}

bool ETFeederNode::involved_dim(int i) {
  const auto& attr = this->get_attr("involved_dim");
  return attr.bool_list().values().at(i);
}

uint32_t ETFeederNode::comm_priority() {
  const auto& attr = this->get_attr("comm_priority");
  return static_cast<uint32_t>(attr.int32_val());
}

uint64_t ETFeederNode::comm_size() {
  const auto& attr = this->get_attr("comm_size");
  return static_cast<uint64_t>(attr.int64_val());
}

uint32_t ETFeederNode::comm_src() {
  const auto& attr = this->get_attr("comm_src");
  return static_cast<uint32_t>(attr.int32_val());
}

uint32_t ETFeederNode::comm_dst() {
  const auto& attr = this->get_attr("comm_dst");
  return static_cast<uint32_t>(attr.int32_val());
}

uint32_t ETFeederNode::comm_tag() {
  const auto& attr = this->get_attr("comm_tag");
  return static_cast<uint32_t>(attr.int32_val());
}
