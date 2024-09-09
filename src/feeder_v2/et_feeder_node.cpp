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

uint64_t ETFeederNode::id() const {
  return this->node_iter_.get_ref().id();
}

std::string ETFeederNode::name() const {
  return this->node_iter_.get_ref().name();
}

bool ETFeederNode::is_cpu_op() const {
  const ChakraProtoMsg::AttributeProto* attr;
  if (this->get_attr("is_cpu_op", &attr)) {
    return this->get_attr_field<bool>(*attr);
  }
  return true;
}

ChakraNodeType ETFeederNode::type() const {
  return this->node_iter_.get_ref().type();
}

uint64_t ETFeederNode::runtime() const {
  return this->node_iter_.get_ref().duration_micros();
}

uint64_t ETFeederNode::num_ops() const {
  return this->get_attr_field<uint64_t>(this->get_attr("num_ops"));
}

uint32_t ETFeederNode::tensor_loc() const {
  return this->get_attr_field<uint32_t>(this->get_attr("tensor_loc"));
}

uint64_t ETFeederNode::tensor_size() const {
  return this->get_attr_field<uint64_t>(this->get_attr("tensor_size"));
}

ChakraCollType ETFeederNode::comm_type() const {
  return static_cast<ChakraCollType>(
      this->get_attr_field<int64_t>(this->get_attr("comm_type")));
}

uint32_t ETFeederNode::comm_priority() const {
  return this->get_attr_field<uint32_t>(this->get_attr("comm_priority"));
}

uint64_t ETFeederNode::comm_size() const {
  return this->get_attr_field<uint64_t>(this->get_attr("comm_size"));
}

uint32_t ETFeederNode::comm_src() const {
  return this->get_attr_field<uint32_t>(this->get_attr("comm_src"));
}

uint32_t ETFeederNode::comm_dst() const {
  return this->get_attr_field<uint32_t>(this->get_attr("comm_dst"));
}

uint32_t ETFeederNode::comm_tag() const {
  return this->get_attr_field<uint32_t>(this->get_attr("comm_tag"));
}

bool ETFeederNode::is_cpu_op(const bool& default_value) const {
  const ChakraProtoMsg::AttributeProto* attr;
  if (this->get_attr("is_cpu_op", &attr))
    return this->get_attr_field<bool>(*attr);
  return default_value;
}
uint64_t ETFeederNode::num_ops(const uint64_t& default_value) const {
  const ChakraProtoMsg::AttributeProto* attr;
  if (this->get_attr("num_ops", &attr))
    return this->get_attr_field<uint64_t>(*attr);
  return default_value;
}
uint32_t ETFeederNode::tensor_loc(const uint32_t& default_value) const {
  const ChakraProtoMsg::AttributeProto* attr;
  if (this->get_attr("tensor_loc", &attr))
    return this->get_attr_field<uint32_t>(*attr);
  return default_value;
}
uint64_t ETFeederNode::tensor_size(const uint64_t& default_value) const {
  const ChakraProtoMsg::AttributeProto* attr;
  if (this->get_attr("tensor_size", &attr))
    return this->get_attr_field<uint64_t>(*attr);
  return default_value;
}
ChakraProtoMsg::CollectiveCommType ETFeederNode::comm_type(
    const ChakraProtoMsg::CollectiveCommType& default_value) const {
  const ChakraProtoMsg::AttributeProto* attr;
  if (this->get_attr("comm_type", &attr))
    return static_cast<ChakraProtoMsg::CollectiveCommType>(
        this->get_attr_field<int64_t>(*attr));
  return default_value;
}
uint32_t ETFeederNode::comm_priority(const uint32_t& default_value) const {
  const ChakraProtoMsg::AttributeProto* attr;
  if (this->get_attr("comm_priority", &attr))
    return this->get_attr_field<uint32_t>(*attr);
  return default_value;
}
uint64_t ETFeederNode::comm_size(const uint64_t& default_value) const {
  const ChakraProtoMsg::AttributeProto* attr;
  if (this->get_attr("comm_size", &attr))
    return this->get_attr_field<uint64_t>(*attr);
  return default_value;
}
uint32_t ETFeederNode::comm_src(const uint32_t& default_value) const {
  const ChakraProtoMsg::AttributeProto* attr;
  if (this->get_attr("comm_src", &attr))
    return this->get_attr_field<uint32_t>(*attr);
  return default_value;
}
uint32_t ETFeederNode::comm_dst(const uint32_t& default_value) const {
  const ChakraProtoMsg::AttributeProto* attr;
  if (this->get_attr("comm_dst", &attr))
    return this->get_attr_field<uint32_t>(*attr);
  return default_value;
}
uint32_t ETFeederNode::comm_tag(const uint32_t& default_value) const {
  const ChakraProtoMsg::AttributeProto* attr;
  if (this->get_attr("comm_tag", &attr))
    return this->get_attr_field<uint32_t>(*attr);
  return default_value;
}

const ChakraAttr& ETFeederNode::get_attr(const std::string& attr_name) const {
  for (const ChakraAttr& attr : this->node_iter_.get_ref().attr())
    if (attr.name() == attr_name)
      return attr;
  throw std::out_of_range("Attribute not found " + attr_name);
}

bool ETFeederNode::get_attr(
    const std::string& attr_name,
    const ChakraProtoMsg::AttributeProto** attr) const {
  for (const ChakraAttr& try_attr : this->node_iter_.get_ref().attr())
    if (try_attr.name() == attr_name) {
      *attr = &try_attr;
      return true;
    }
  return false;
}

// template <typename T>
// T ETFeederNode::get_attr_field(
//     const ChakraProtoMsg::AttributeProto& attr) const {
//   constexpr bool STRONG_TYPED = true;

//   switch (attr.value_case()) {
//     case ChakraAttr::kDoubleVal:
//       if constexpr (STRONG_TYPED && (!std::is_same<T, double>::value)) {
//         throw std::invalid_argument("Attribute type not supported");
//       }
//       return static_cast<T>(attr.double_val());
//     case ChakraAttr::kFloatVal:
//       if constexpr (STRONG_TYPED && (!std::is_same<T, float>::value)) {
//         throw std::invalid_argument("Attribute type not supported");
//       }
//       return static_cast<T>(attr.float_val());
//     case ChakraAttr::kInt32Val:
//       if constexpr (STRONG_TYPED && (!std::is_same<T, int32_t>::value)) {
//         throw std::invalid_argument("Attribute type not supported");
//       }
//       return static_cast<T>(attr.int32_val());
//     case ChakraAttr::kInt64Val:
//       if constexpr (STRONG_TYPED && (!std::is_same<T, int64_t>::value)) {
//         throw std::invalid_argument("Attribute type not supported");
//       }
//       return static_cast<T>(attr.int64_val());
//     case ChakraAttr::kUint32Val:
//       if constexpr (STRONG_TYPED && (!std::is_same<T, uint32_t>::value)) {
//         throw std::invalid_argument("Attribute type not supported");
//       }
//       return static_cast<T>(attr.uint32_val());
//     case ChakraAttr::kUint64Val:
//       if constexpr (STRONG_TYPED && (!std::is_same<T, uint64_t>::value)) {
//         throw std::invalid_argument("Attribute type not supported");
//       }
//       return static_cast<T>(attr.uint64_val());
//     case ChakraAttr::kSint32Val:
//       if constexpr (STRONG_TYPED && (!std::is_same<T, int32_t>::value)) {
//         throw std::invalid_argument("Attribute type not supported");
//       }
//       return static_cast<T>(attr.sint32_val());
//     case ChakraAttr::kSint64Val:
//       if constexpr (STRONG_TYPED && (!std::is_same<T, int64_t>::value)) {
//         throw std::invalid_argument("Attribute type not supported");
//       }
//       return static_cast<T>(attr.sint64_val());
//     case ChakraAttr::kFixed32Val:
//       if constexpr (STRONG_TYPED && (!std::is_same<T, uint32_t>::value)) {
//         throw std::invalid_argument("Attribute type not supported");
//       }
//       return static_cast<T>(attr.fixed32_val());
//     case ChakraAttr::kFixed64Val:
//       if constexpr (STRONG_TYPED && (!std::is_same<T, uint64_t>::value)) {
//         throw std::invalid_argument("Attribute type not supported");
//       }
//       return static_cast<T>(attr.fixed64_val());
//     case ChakraAttr::kSfixed32Val:
//       if constexpr (STRONG_TYPED && (!std::is_same<T, int32_t>::value)) {
//         throw std::invalid_argument("Attribute type not supported");
//       }
//       return static_cast<T>(attr.sfixed32_val());
//     case ChakraAttr::kSfixed64Val:
//       if constexpr (STRONG_TYPED && (!std::is_same<T, int64_t>::value)) {
//         throw std::invalid_argument("Attribute type not supported");
//       }
//       return static_cast<T>(attr.sfixed64_val());
//     case ChakraAttr::kBoolVal:
//       if constexpr (STRONG_TYPED && (!std::is_same<T, bool>::value)) {
//         throw std::invalid_argument("Attribute type not supported");
//       }
//       return static_cast<T>(attr.bool_val());
//     default:
//       throw std::invalid_argument("Attribute type not supported");
//   }
// }

template <typename T>
T ETFeederNode::get_attr_field(
    const ChakraProtoMsg::AttributeProto& attr) const {
  constexpr bool STRONG_TYPED = false;

  // Helper function to handle type checking
  auto get_value = [](auto value, auto expected_type) -> T {
    if constexpr (
        STRONG_TYPED && (!std::is_same<T, decltype(expected_type)>::value)) {
      throw std::invalid_argument("Attribute type not supported");
    }
    return static_cast<T>(value);
  };

  switch (attr.value_case()) {
    case ChakraAttr::kDoubleVal:
      return get_value(attr.double_val(), double());
    case ChakraAttr::kFloatVal:
      return get_value(attr.float_val(), float());
    case ChakraAttr::kInt32Val:
      return get_value(attr.int32_val(), int32_t());
    case ChakraAttr::kInt64Val:
      return get_value(attr.int64_val(), int64_t());
    case ChakraAttr::kUint32Val:
      return get_value(attr.uint32_val(), uint32_t());
    case ChakraAttr::kUint64Val:
      return get_value(attr.uint64_val(), uint64_t());
    case ChakraAttr::kSint32Val:
      return get_value(attr.sint32_val(), int32_t());
    case ChakraAttr::kSint64Val:
      return get_value(attr.sint64_val(), int64_t());
    case ChakraAttr::kFixed32Val:
      return get_value(attr.fixed32_val(), uint32_t());
    case ChakraAttr::kFixed64Val:
      return get_value(attr.fixed64_val(), uint64_t());
    case ChakraAttr::kSfixed32Val:
      return get_value(attr.sfixed32_val(), int32_t());
    case ChakraAttr::kSfixed64Val:
      return get_value(attr.sfixed64_val(), int64_t());
    case ChakraAttr::kBoolVal:
      return get_value(attr.bool_val(), bool());
    default:
      throw std::invalid_argument("Attribute type not supported");
  }
}