#ifndef DB_AS_MAP__UNORDERED_MAP
#define DB_AS_MAP__UNORDERED_MAP

#include <sqlite3.h>
#include <stdexcept>
#include <string>

namespace db_as_map {

template <class Key, class Value>
class unordered_map {
 private:
  std::string db_name;
  sqlite3* db;
  std::string table_name;
  static uint64_t _table_cnt;
  void create_table();
  void sqlite3_bind_value(sqlite3_stmt*, int, const Value&);
  void sqlite3_bind_key(sqlite3_stmt*, int, const Key&);

 public:
  unordered_map(const std::string& db_name);
  unordered_map();
  ~unordered_map();

  void insert(const Key& key, const Value& value);

  bool erase(const Key& key);

  bool in(const Key& key);

  Value at(const Key& key);

  void replace(const Key& key, const Value& value);

  void set(const Key& key, const Value& value);

  void sync(void);
};

} // namespace db_as_map

#include "db_as_map/unordered_map_impl.hh"

#endif
