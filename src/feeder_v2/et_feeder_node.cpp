#include "et_feeder_node.h"

using namespace Chakra;

std::shared_ptr<ChakraProtoMsg::Node> ETFeederNode::getChakraNode() {
  return std::make_shared<ChakraProtoMsg::Node>(node_iter_.get_ref());
}

const ChakraProtoMsg::AttributeProto& ETFeederNode::get_other_attr(
    const std::string& attr_name) const {
  return this->get_attr(attr_name);
}

bool ETFeederNode::has_other_attr(const std::string& attr_name) const {
  const ChakraProtoMsg::AttributeProto* attr;
  return this->get_attr(attr_name, &attr);
}

uint64_t ETFeederNode::id() const { return this->node_iter_.get_ref().id(); }

std::string ETFeederNode::name() const {
  return this->node_iter_.get_ref().name();
}

bool ETFeederNode::is_cpu_op() const {
  const ChakraProtoMsg::AttributeProto* attr;
  if (this->get_attr("is_cpu_op", &attr)) return attr->bool_val();
  return true;
}

ChakraNodeType ETFeederNode::type() const {
  return this->node_iter_.get_ref().type();
}

uint64_t ETFeederNode::runtime() const {
  return this->node_iter_.get_ref().duration_micros();
}

uint64_t ETFeederNode::num_ops() const {
  return this->get_attr("num_ops").int64_val();
}

uint32_t ETFeederNode::tensor_loc() const {
  return this->get_attr("tensor_loc").int32_val();
}

uint64_t ETFeederNode::tensor_size() const {
  return this->get_attr("tensor_size").uint64_val();
}

ChakraCollType ETFeederNode::comm_type() const {
  return static_cast<ChakraCollType>(this->get_attr("comm_type").int64_val());
}

uint32_t ETFeederNode::comm_priority() const {
  return this->get_attr("comm_priority").int32_val();
}

uint64_t ETFeederNode::comm_size() const {
  return this->get_attr("comm_size").int64_val();
}

uint32_t ETFeederNode::comm_src() const {
  return this->get_attr("comm_src").int32_val();
}

uint32_t ETFeederNode::comm_dst() const {
  return this->get_attr("comm_dst").int32_val();
}

uint32_t ETFeederNode::comm_tag() const {
  return this->get_attr("comm_tag").int32_val();
}

const ChakraAttr& ETFeederNode::get_attr(const std::string& attr_name) const {
  for (const ChakraAttr& attr : this->node_iter_.get_ref().attr())
    if (attr.name() == attr_name) return attr;
  throw std::runtime_error("Attribute not found");
}

bool ETFeeerNode::get_attr(const std::string& attr_name,
                           const ChakraProtoMsg::AttributeProto** attr) const {
  for (const ChakraAttr& try_attr : this->node_iter_.get_ref().attr())
    if (try_attr.name() == attr_name) {
      *attr = &try_attr;
      return true;
    }
  return false;
}
