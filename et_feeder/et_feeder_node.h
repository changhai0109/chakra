#pragma once

#include <memory>
#include <unordered_set>
#include <vector>

#include "et_def/et_def.pb.h"

namespace Chakra {

class ETFeederNode {
 public:
  ETFeederNode(std::shared_ptr<ChakraProtoMsg::Node> node);
  std::shared_ptr<ChakraProtoMsg::Node> getChakraNode();
  void addChild(std::shared_ptr<ETFeederNode> node);
  std::vector<std::shared_ptr<ETFeederNode>> getChildren();
  void addDepUnresolvedParentID(uint64_t node_id);
  std::vector<uint64_t> getDepUnresolvedParentIDs();
  void setDepUnresolvedParentIDs(
      std::vector<uint64_t> const& dep_unresolved_parent_ids);

  uint64_t id();
  std::string name();
  bool is_cpu_op();
  ChakraProtoMsg::NodeType type();
  uint64_t runtime();
  uint64_t num_ops();
  uint32_t tensor_loc();
  uint64_t tensor_size();
  ChakraProtoMsg::CollectiveCommType comm_type();
  uint32_t involved_dim_size();
  bool involved_dim(int i);
  uint32_t comm_priority();
  uint64_t comm_size();
  uint32_t comm_src();
  uint32_t comm_dst();
  uint32_t comm_tag();

  template <typename T>
  bool getAttr(const string& attr_name, T* value);

  template <typename T>
  void getAttr(const string& attr_name, T* value, T& defaultValue);

 private:
  class _AttrTypeConverter {
    template <typename T>
    static bool read_from_attr(
        ChakraProtoMsg::AttributeProto& attr,
        T* value,
#ifndef STRICT_TYPE_ENABLED
        bool strict_type = false
#else
        bool strict_type = true
#endif);
    template <>
    static bool read_from_attr<int32_t>(
        ChakraProtoMsg::AttributeProto& attr,
        int32_t* value,
        bool strict_type);
    template <>
    static bool read_from_attr<int64_t>(
        ChakraProtoMsg::AttributeProto& attr,
        int64_t* value,
        bool strict_type);
    template <>
    static bool read_from_attr<uint32_t>(
        ChakraProtoMsg::AttributeProto& attr,
        uint32_t* value,
        bool strict_type);
    template <>
    static bool read_from_attr<uint64_t>(
        ChakraProtoMsg::AttributeProto& attr,
        uint64_t* value,
        bool strict_type);
    template <>
    static bool read_from_attr<bool>(
        ChakraProtoMsg::AttributeProto& attr,
        bool* value,
        bool strict_type);
    template <>
    static bool read_from_attr<std::string>(
        ChakraProtoMsg::AttributeProto& attr,
        std::string* value,
        bool strict_type);
    template <>
    static bool read_from_attr<std::vector<int32_t>>(
        ChakraProtoMsg::AttributeProto& attr,
        std::vector<int32_t>* value,
        bool strict_type);
    template <>
    static bool read_from_attr<std::vector<int64_t>>(
        ChakraProtoMsg::AttributeProto& attr,
        std::vector<int64_t>* value,
        bool strict_type);
    template <>
    static bool read_from_attr<std::vector<uint32_t>>(
        ChakraProtoMsg::AttributeProto& attr,
        std::vector<uint32_t>* value,
        bool strict_type);
    template <>
    static bool read_from_attr<std::vector<uint64_t>>(
        ChakraProtoMsg::AttributeProto& attr,
        std::vector<uint64_t>* value,
        bool strict_type);
    template <>
    static bool read_from_attr<std::vector<bool>>(
        ChakraProtoMsg::AttributeProto& attr,
        std::vector<bool>* value,
        bool strict_type);
    template <>
    static bool read_from_attr<std::vector<std::string>>(
        ChakraProtoMsg::AttributeProto& attr,
        std::vector<std::string>* value,
        bool strict_type);
  };
  void assign_attr_val(
      std::shared_ptr<ChakraProtoMsg::Node> node,
      int i,
      void* member);

  std::shared_ptr<ChakraProtoMsg::Node> node_{nullptr};
  std::unordered_set<std::shared_ptr<ETFeederNode>> children_set_{};
  std::vector<std::shared_ptr<ETFeederNode>> children_vec_{};
  std::vector<uint64_t> dep_unresolved_parent_ids_{};
  std::unordered_map<std::string, ChakraProtoMsg::AttributeProto&> attrs_{};

  uint64_t id_;
  std::string name_;
  // uint32_t is_cpu_op_;
  uint64_t runtime_;
  // uint64_t num_ops_;
  // uint32_t tensor_loc_;
  // uint64_t tensor_size_;
  // ChakraProtoMsg::CollectiveCommType comm_type_;
  // uint32_t involved_dim_size_;
  // std::vector<bool> involved_dim_;
  // uint32_t comm_priority_;
  // uint64_t comm_size_;
  // uint32_t comm_src_;
  // uint32_t comm_dst_;
  // uint32_t comm_tag_;
};

} // namespace Chakra
