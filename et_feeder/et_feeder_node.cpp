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
    this->attrs_[attr_name] = attr;
  }
}

template <typename T>
bool ETFeederNode::getAttr(const string& attr_name, T* value) {
  auto& item = this->attrs_.find(attr_name);
  if (item == this->attrs_.end())
    return false;
  auto& attr = item.second;
  return ETFeederNode::_AttrTypeConverter::read_from_attr(attr, value);
}

template <typename T>
void ETFeederNode::getAttr(const string& attr_name, T* value, T& defaultValue) {
  if (!ETFeederNode::getAttr(attr_name, value))
    *value = defaultValue;
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
  bool ret = false;
  this->getAttr("is_cpu_op", &ret, true);
  return ret;
}

ChakraProtoMsg::NodeType ETFeederNode::type() {
  return node_->type();
}

uint64_t ETFeederNode::runtime() {
  return runtime_;
}

uint64_t ETFeederNode::num_ops() {
  uint64_t ret = 0ul;
  if (!this->getAttr("num_ops", &ret))
    assert(false);
  return ret;
}

uint32_t ETFeederNode::tensor_loc() {
  uint32_t ret = 0u;
  if (!this->getAttr("tensor_loc", &ret))
    assert(false);
  return ret;
}

uint64_t ETFeederNode::tensor_size() {
  uint64_t ret = 0u;
  if (!this->getAttr("tensor_size", &ret))
    assert(false);
  return ret;
}

ChakraProtoMsg::CollectiveCommType ETFeederNode::comm_type() {
  int64_t ret = 0l;
  if (!this->getAttr("comm_type", &ret))
    assert(false);
  return static_cast<ChakraProtoMsg::CollectiveCommType>(ret);
}

uint32_t ETFeederNode::involved_dim_size() {
  std::vector<bool> involved_dim;
  if (!this->getAttr("involved_dim", &involved_dim))
    assert(false);
  return static_cast<uint32_t>(involved_dim.size());
}

bool ETFeederNode::involved_dim(int i) {
  std::vector<bool> involved_dim;
  if (!this->getAttr("involved_dim", &involved_dim))
    assert(false);
  if (i >= involved_dim.size())
    assert(false);
  return involved_dim.at(i);
}

uint32_t ETFeederNode::comm_priority() {
  uint32_t ret = 0u;
  if (!this->getAttr("comm_priority", &ret))
    assert(false);
  return ret;
}

uint64_t ETFeederNode::comm_size() {
  uint64_t ret = 0ul;
  if (!this->getAttr("comm_size", &ret))
    assert(false);
  return ret;
}

uint32_t ETFeederNode::comm_src() {
  uint32_t ret = 0u;
  if (!this->getAttr("comm_src", &ret))
    assert(false);
  return ret;
}

uint32_t ETFeederNode::comm_dst() {
  uint32_t ret = 0u;
  if (!this->getAttr("comm_dst", &ret))
    assert(false);
  return ret;
}

uint32_t ETFeederNode::comm_tag() {
  uint32_t ret = 0u;
  if (!this->getAttr("comm_tag", &ret))
    assert(false);
  return ret;
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
bool ETFeederNode::_AttrTypeConverter::read_from_attr<int32_t>(
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
bool ETFeederNode::_AttrTypeConverter::read_from_attr<int64_t>(
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
bool ETFeederNode::_AttrTypeConverter::read_from_attr<uint32_t>(
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
bool ETFeederNode::_AttrTypeConverter::read_from_attr<uint64_t>(
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
bool ETFeederNode::_AttrTypeConverter::read_from_attr<bool>(
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
bool ETFeederNode::_AttrTypeConverter::read_from_attr<std::string>(
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

template <>
bool ETFeederNode::_AttrTypeConverter::read_from_attr<std::vector<int32_t>>(
    ChakraProtoMsg::AttributeProto& attr,
    std::vector<int32_t>* value,
    bool strict_type) {
  if (value == nullptr) {
    std::cerr << "value should be initialized outside the function"
              << std::endl;
    exit(EXIT_FAILURE);
  }
  value->clear();
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kInt32List) {
    for (auto& item : attr.int32_list().values())
      value->push_back(item);
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kSint32List) {
    for (auto& item : attr.sint32_list().values())
      value->push_back(item);
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kSfixed32List) {
    for (auto& item : attr.sfixed32_list().values())
      value->push_back(item);
    return true;
  }
  if (!strict_type) {
    bool success = false;
    std::vector<uint32_t> value_u32;
    std::vector<uint64_t> value_u64;
    std::vector<int32_t> value_32;
    std::vector<int64_t> value_64;
    // try int64
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_64, true);
    if (success) {
      for (auto& item : value_64)
        value->push_back(static_cast<int32_t>(item));
      return success;
    }
    // try uint32
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u32, true);
    if (success) {
      for (auto& item : value_u32)
        value->push_back(static_cast<int32_t>(item));
      return success;
    }
    // try uint64
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u64, true);
    if (success) {
      for (auto& item : value_u64)
        value->push_back(static_cast<int32_t>(item));
      return success;
    }
  }
  return false;
}

template <>
bool ETFeederNode::_AttrTypeConverter::read_from_attr<std::vector<int64_t>>(
    ChakraProtoMsg::AttributeProto& attr,
    std::vector<int64_t>* value,
    bool strict_type) {
  if (value == nullptr) {
    std::cerr << "value should be initialized outside the function"
              << std::endl;
    exit(EXIT_FAILURE);
  }
  value->clear();
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kInt64List) {
    for (auto& item : attr.int64_list().values())
      value->push_back(item);
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kSint64List) {
    for (auto& item : attr.sint64_list().values())
      value->push_back(item);
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kSfixed64List) {
    for (auto& item : attr.sfixed64_list().values())
      value->push_back(item);
    return true;
  }
  if (!strict_type) {
    bool success = false;
    std::vector<uint32_t> value_u32;
    std::vector<uint64_t> value_u64;
    std::vector<int32_t> value_32;
    std::vector<int64_t> value_64;
    // try int32
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_32, true);
    if (success) {
      for (auto& item : value_32)
        value->push_back(static_cast<int64_t>(item));
      return success;
    }
    // try uint32
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u32, true);
    if (success) {
      for (auto& item : value_u32)
        value->push_back(static_cast<int64_t>(item));
      return success;
    }
    // try uint64
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u64, true);
    if (success) {
      for (auto& item : value_u64)
        value->push_back(static_cast<int64_t>(item));
      return success;
    }
  }
  return false;
}

template <>
bool ETFeederNode::_AttrTypeConverter::read_from_attr<std::vector<uint32_t>>(
    ChakraProtoMsg::AttributeProto& attr,
    std::vector<uint32_t>* value,
    bool strict_type) {
  if (value == nullptr) {
    std::cerr << "value should be initialized outside the function"
              << std::endl;
    exit(EXIT_FAILURE);
  }
  value->clear();
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kUint32List) {
    for (auto& item : attr.uint32_list().values())
      value->push_back(item);
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kFixed32List) {
    for (auto& item : attr.fixed32_list().values())
      value->push_back(item);
    return true;
  }
  if (!strict_type) {
    bool success = false;
    std::vector<uint32_t> value_u32;
    std::vector<uint64_t> value_u64;
    std::vector<int32_t> value_32;
    std::vector<int64_t> value_64;
    // try int32
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_32, true);
    if (success) {
      for (auto& item : value_32)
        value->push_back(static_cast<uint32_t>(item));
      return success;
    }
    // try int64
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_64, true);
    if (success) {
      for (auto& item : value_64)
        value->push_back(static_cast<uint32_t>(item));
      return success;
    }
    // try uint64
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u64, true);
    if (success) {
      for (auto& item : value_u64)
        value->push_back(static_cast<uint32_t>(item));
      return success;
    }
  }
  return false;
}

template <>
bool ETFeederNode::_AttrTypeConverter::read_from_attr<std::vector<uint64_t>>(
    ChakraProtoMsg::AttributeProto& attr,
    std::vector<uint64_t>* value,
    bool strict_type) {
  if (value == nullptr) {
    std::cerr << "value should be initialized outside the function"
              << std::endl;
    exit(EXIT_FAILURE);
  }
  value->clear();
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kUint32List) {
    for (auto& item : attr.uint64_list().values())
      value->push_back(item);
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kFixed32List) {
    for (auto& item : attr.fixed64_list().values())
      value->push_back(item);
    return true;
  }
  if (!strict_type) {
    bool success = false;
    std::vector<uint32_t> value_u32;
    std::vector<uint64_t> value_u64;
    std::vector<int32_t> value_32;
    std::vector<int64_t> value_64;
    // try int32
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_32, true);
    if (success) {
      for (auto& item : value_32)
        value->push_back(static_cast<uint64_t>(item));
      return success;
    }
    // try int64
    success =
        ETFeederNode::_AttrTypeConverter::read_from_attr(attr, &value_64, true);
    if (success) {
      for (auto& item : value_64)
        value->push_back(static_cast<uint64_t>(item));
      return success;
    }
    // try uint32
    success = ETFeederNode::_AttrTypeConverter::read_from_attr(
        attr, &value_u32, true);
    if (success) {
      for (auto& item : value_u32)
        value->push_back(static_cast<uint64_t>(item));
      return success;
    }
  }
  return false;
}

template <>
bool ETFeederNode::_AttrTypeConverter::read_from_attr<std::vector<bool>>(
    ChakraProtoMsg::AttributeProto& attr,
    std::vector<bool>* value,
    bool strict_type) {
  if (value == nullptr) {
    std::cerr << "value should be initialized outside the function"
              << std::endl;
    exit(EXIT_FAILURE);
  }
  value->clear();
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kBoolList) {
    for (auto& item : attr.bool_list().values())
      value->push_back(item);
    return true;
  }
  return false;
}

template <>
bool ETFeederNode::_AttrTypeConverter::read_from_attr<std::vector<std::string>>(
    ChakraProtoMsg::AttributeProto& attr,
    std::vector<std::string>* value,
    bool strict_type) {
  if (value == nullptr) {
    std::cerr << "value should be initialized outside the function"
              << std::endl;
    exit(EXIT_FAILURE);
  }
  value->clear();
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kStringList) {
    for (auto& item : attr.string_list().values())
      value->push_back(item);
    return true;
  }
  if (attr.value_case() == ChakraProtoMsg::AttributeProto::kBytesList) {
    for (auto& item : attr.bytes_list().values())
      value->push_back(item);
    return true;
  }
  return false;
}
