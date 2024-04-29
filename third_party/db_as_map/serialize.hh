#ifndef DB_AS_MAP__SERIALIZE
#define DB_AS_MAP__SERIALIZE

#include <cstddef>
#include <string>

namespace db_as_map {

template <typename T>
struct Serialize;

#define _DB_AS_MAP_SERIALIZE(T)                       \
  template <>                                         \
  struct Serialize<T> {                               \
    static void* serialize(const T& value) {          \
      return (void*)&value;                           \
    }                                                 \
    static size_t size(const T& value) {              \
      return sizeof(value);                           \
    }                                                 \
    static T deserialize(const void* value, size_t) { \
      return *(T*)value;                              \
    }                                                 \
  };

_DB_AS_MAP_SERIALIZE(bool);
_DB_AS_MAP_SERIALIZE(char);
_DB_AS_MAP_SERIALIZE(signed char);
_DB_AS_MAP_SERIALIZE(unsigned char);
_DB_AS_MAP_SERIALIZE(wchar_t);
_DB_AS_MAP_SERIALIZE(char16_t);
_DB_AS_MAP_SERIALIZE(char32_t);
_DB_AS_MAP_SERIALIZE(short);
_DB_AS_MAP_SERIALIZE(int);
_DB_AS_MAP_SERIALIZE(long);
_DB_AS_MAP_SERIALIZE(long long);
_DB_AS_MAP_SERIALIZE(unsigned short);
_DB_AS_MAP_SERIALIZE(unsigned int);
_DB_AS_MAP_SERIALIZE(unsigned long);
_DB_AS_MAP_SERIALIZE(unsigned long long);
_DB_AS_MAP_SERIALIZE(float);
_DB_AS_MAP_SERIALIZE(double);
_DB_AS_MAP_SERIALIZE(long double);

#undef _DB_AS_MAP_SERIALIZE

template <>
struct Serialize<std::string> {
  static void* serialize(const std::string& value) {
    return (void*)value.c_str();
  }
  static size_t size(const std::string& value) { return value.size(); }
  static std::string deserialize(const void* value, size_t size) {
    return std::string((const char*)value, size);
  }
};

}  // namespace db_as_map

#endif  // DB_AS_MAP__SERIALIZE
