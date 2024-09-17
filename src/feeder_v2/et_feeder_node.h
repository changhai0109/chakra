#ifndef CHAKRA__ET_OPERATOR__ET_FEEDER_NODE_H
#define CHAKRA__ET_OPERATOR__ET_FEEDER_NODE_H

#include <memory>

#include "et_def.pb.h"
#include "raw_et_operator.h"

using ChakraAttr = ChakraProtoMsg::AttributeProto;
using ChakraNode = ChakraProtoMsg::Node;
using ChakraCollType = ChakraProtoMsg::CollectiveCommType;
using ChakraNodeType = ChakraProtoMsg::NodeType;

namespace Chakra {
class ETFeederNode {
 public:
  ETFeederNode(const ETOperator::Iterator& node_iter) : node_iter_(node_iter) {}
  const std::weak_ptr<const ChakraProtoMsg::Node> getChakraNode();
  void addChild(std::shared_ptr<ETFeederNode> node) = delete;
  std::vector<std::shared_ptr<ETFeederNode>> getChildren() = delete;
  std::vector<std::shared_ptr<ETFeederNode>> getDataChildren();
  void addDepUnresolvedParentID(uint64_t node_id) = delete;
  std::vector<uint64_t> getDepUnresolvedParentIDs() = delete;
  void setDepUnresolvedParentIDs(
      std::vector<uint64_t> const& dep_unresolved_parent_ids) = delete;
  const ChakraProtoMsg::AttributeProto get_other_attr(
      const std::string& attr_name) const;
  bool has_other_attr(const std::string& attr_name) const;

  uint64_t id() const;
  std::string name() const;
  bool is_cpu_op() const;
  ChakraProtoMsg::NodeType type() const;
  uint64_t runtime() const;
  uint64_t num_ops() const;
  uint32_t tensor_loc() const;
  uint64_t tensor_size() const;
  ChakraProtoMsg::CollectiveCommType comm_type() const;
  uint32_t comm_priority() const;
  uint64_t comm_size() const;
  uint32_t comm_src() const;
  uint32_t comm_dst() const;
  uint32_t comm_tag() const;

  bool is_cpu_op(const bool& default_value) const;
  uint64_t num_ops(const uint64_t& default_value) const;
  uint32_t tensor_loc(const uint32_t& default_value) const;
  uint64_t tensor_size(const uint64_t& default_value) const;
  ChakraProtoMsg::CollectiveCommType comm_type(
      const ChakraProtoMsg::CollectiveCommType& default_value) const;
  uint32_t comm_priority(const uint32_t& default_value) const;
  uint64_t comm_size(const uint64_t& default_value) const;
  uint32_t comm_src(const uint32_t& default_value) const;
  uint32_t comm_dst(const uint32_t& default_value) const;
  uint32_t comm_tag(const uint32_t& default_value) const;

 private:
  const ChakraAttr get_attr(const std::string& attr_name) const;
  bool get_attr(
      const std::string& attr_name,
      const ChakraProtoMsg::AttributeProto** attr) const;
  template <typename T>
  T get_attr_field(const ChakraProtoMsg::AttributeProto& attr) const;
  ETOperator::Iterator node_iter_;
};
} // namespace Chakra

#endif
