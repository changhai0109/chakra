#include "et_feeder/et_feeder_node.h"

using namespace std;
using namespace Chakra;

ETFeederNode::ETFeederNode(std::shared_ptr<ChakraProtoMsg::Node> node) {
  this->node_ = node;
  this->id_ = node->id();
  this->name_ = node->name();
  this->runtime_ = node->duration_micros();
  // this->is_cpu_op_ = 1;

  for (const auto& attr : node->attr()) {
    const string& attr_name = attr.name();
    this->attrs_[attr_name] = attr;
    // if (attr_name == "is_cpu_op") {
    //   this->is_cpu_op_ = static_cast<uint32_t>(attr.int32_val());
    // } else if (attr_name == "num_ops") {
    //   this->num_ops_ = static_cast<uint64_t>(attr.int64_val());
    // } else if (attr_name == "tensor_size") {
    //   this->tensor_size_ = attr.uint64_val();
    // } else if (attr_name == "comm_type") {
    //   this->comm_type_ =
    //       static_cast<ChakraProtoMsg::CollectiveCommType>(attr.int64_val());
    // } else if (attr_name == "involved_dim") {
    //   this->involved_dim_.clear();
    //   for (const bool val : attr.bool_list().values()) {
    //     this->involved_dim_.push_back(val);
    //   }
    //   this->involved_dim_size_ = this->involved_dim_.size();
    // } else if (attr_name == "comm_priority") {
    //   this->comm_priority_ = static_cast<uint32_t>(attr.int32_val());
    // } else if (attr_name == "comm_size") {
    //   this->comm_size_ = attr.int64_val();
    // } else if (attr_name == "comm_src") {
    //   this->comm_src_ = static_cast<uint32_t>(attr.int32_val());
    // } else if (attr_name == "comm_dst") {
    //   this->comm_dst_ = static_cast<uint32_t>(attr.int32_val());
    // } else if (attr_name == "comm_tag") {
    //   this->comm_tag_ = static_cast<uint32_t>(attr.int32_val());
    // }
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

uint64_t ETFeederNode::id() {
  return id_;
}

string ETFeederNode::name() {
  return name_;
}

bool ETFeederNode::is_cpu_op() {
  return is_cpu_op_;
}

ChakraProtoMsg::NodeType ETFeederNode::type() {
  return node_->type();
}

uint64_t ETFeederNode::runtime() {
  return runtime_;
}

uint64_t ETFeederNode::num_ops() {
  return num_ops_;
}

uint32_t ETFeederNode::tensor_loc() {
  return tensor_loc_;
}

uint64_t ETFeederNode::tensor_size() {
  return tensor_size_;
}

ChakraProtoMsg::CollectiveCommType ETFeederNode::comm_type() {
  return comm_type_;
}

uint32_t ETFeederNode::involved_dim_size() {
  return involved_dim_size_;
}

bool ETFeederNode::involved_dim(int i) {
  return involved_dim_[i];
}

uint32_t ETFeederNode::comm_priority() {
  return comm_priority_;
}

uint64_t ETFeederNode::comm_size() {
  return comm_size_;
}

uint32_t ETFeederNode::comm_src() {
  return comm_src_;
}

uint32_t ETFeederNode::comm_dst() {
  return comm_dst_;
}

uint32_t ETFeederNode::comm_tag() {
  return comm_tag_;
}

template <typename T>
bool ETFeederNode::_AttrTypeConverter::read_from_attr(
    ChakraProtoMsg::AttributeProto& attr,
    T* value,
#ifndef ETFEEDERNODE_ATTRTYPE_STRICT
    bool strict_type = false
#else
    bool strict_type = true
#endif
) {
  return false;
}

template <>
bool ETFeederNode::_AttrTypeConverter::read_from_attr(
    ChakraProtoMsg::AttributeProto& attr,
    int32_t* value,
    bool strict_type) {
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kInt32Val) {
    *value = attr.int32_val();
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kSint32Val) {
    *value = attr.sint32_val();
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kSfixed32Val) {
    *value = attr.sfixed32_val();
    return true;
  }
  if (!strict_type) {
    int64_t value_64 = 0l;
    uint64_t value_u64 = 0ul;
    uint32_t value_u32 = 0u;
    bool success = false;
    // try int64
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_64, true);
    if (success) {
      *value = static_cast<int32_t>(value_64);
      return success;
    }
    // try uint64
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u64, true);
    if (success) {
      *value = static_cast<int32_t>(value_u64);
      return success;
    }
    // try uint32
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u32, true);
    if (success) {
      *value = static_cast<int32_t>(value_u32);
      return success;
    }
  }
  return false;
}

template <>
bool ETFeederNode::_AttrTypeConverter::read_from_attr(
    ChakraProtoMsg::AttributeProto& attr,
    int64_t* value,
    bool strict_type) {
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kInt64Val) {
    *value = attr.int64_val();
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kSint64Val) {
    *value = attr.sint64_val();
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kSfixed64Val) {
    *value = attr.sfixed64_val();
    return true;
  }
  if (!strict_type) {
    int32_t value_32 = 0;
    uint64_t value_u64 = 0ul;
    uint32_t value_u32 = 0u;
    bool success = false;
    // try int32
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_32, true);
    if (success) {
      *value = static_cast<int64_t>(value_32);
      return success;
    }
    // try uint64
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u64, true);
    if (success) {
      *value = static_cast<int64_t>(value_u64);
      return success;
    }
    // try uint32
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u32, true);
    if (success) {
      *value = static_cast<int64_t>(value_u32);
      return success;
    }
  }
  return false;
}

template <>
bool ETFeederNode::_AttrTypeConverter::read_from_attr(
    ChakraProtoMsg::AttributeProto& attr,
    uint32_t* value,
    bool strict_type) {
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kUint32Val) {
    *value = attr.uint32_val();
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kFixed32Val) {
    *value = attr.fixed32_val();
    return true;
  }
  if (!strict_type) {
    int64_t value_64 = 0l;
    int32_t value_32 = 0;
    uint64_t value_u64 = 0ul;
    bool success = false;
    // try int64
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_64, true);
    if (success) {
      *value = static_cast<uint32_t>(value_64);
      return success;
    }
    // try int32
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_32, true);
    if (success) {
      *value = static_cast<uint32_t>(value_32);
      return success;
    }
    // try uint64
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u64, true);
    if (success) {
      *value = static_cast<uint32_t>(value_u64);
      return success;
    }
  }
  return false;
}

template <>
bool ETFeederNode::_AttrTypeConverter::read_from_attr(
    ChakraProtoMsg::AttributeProto& attr,
    uint64_t* value,
    bool strict_type) {
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kUint64Val) {
    *value = attr.uint64_val();
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kFixed64Val) {
    *value = attr.fixed64_val();
    return true;
  }
  if (!strict_type) {
    int64_t value_64 = 0l;
    int32_t value_32 = 0;
    uint32_t value_u32 = 0u;
    bool success = false;
    // try int64
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_64, true);
    if (success) {
      *value = static_cast<uint64_t>(value_64);
      return success;
    }
    // try int32
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_32, true);
    if (success) {
      *value = static_cast<uint64_t>(value_32);
      return success;
    }
    // try uint32
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u32, true);
    if (success) {
      *value = static_cast<uint64_t>(value_u32);
      return success;
    }
  }
  return false;
}

template <>
bool ETFeederNode::_AttrTypeConverter::read_from_attr(
    ChakraProtoMsg::AttributeProto& attr,
    bool* value,
    bool strict_type) {
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kBoolVal) {
    *value = attr.bool_val();
    return true;
  }
  if (!strict_type) {
    int64_t value_64 = 0l;
    int32_t value_32 = 0;
    uint64_t value_u64 = 0ul;
    uint32_t value_u32 = 0u;
    bool success = false;
    // try int64
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_64, true);
    if (success) {
      *value = static_cast<bool>(value_64);
      return success;
    }
    // try int32
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_32, true);
    if (success) {
      *value = static_cast<bool>(value_32);
      return success;
    }
    // try uint64
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u64, true);
    if (success) {
      *value = static_cast<bool>(value_u64);
      return success;
    }
    // try uint32
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u32, true);
    if (success) {
      *value = static_cast<bool>(value_u32);
      return success;
    }
  }
  return false;
}

template <>
bool ETFeederNode::_AttrTypeConverter::read_from_attr(
    ChakraProtoMsg::AttributeProto& attr,
    std::string* value,
    bool strict_type) {
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kStringVal) {
    *value = attr.string_val();
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kBytesVal) {
    *value = attr.bytes_val();
    return true;
  }
  return false;
}
