#include <sqlite3.h>
#include <unistd.h>
#include <string>
#include <type_traits>
#include "db_as_map/serialize.hh"
#include "db_as_map/unordered_map.hh"

template <class Key, class Value>
uint64_t db_as_map::unordered_map<Key, Value>::_table_cnt = 0ul;

static std::string default_db_name = []() {
  char buffer[] = "/tmp/db_as_map_XXXXXX.db";
  int tempFileDescriptor = mkstemps64(buffer, 3);
  if (tempFileDescriptor == -1) {
    throw std::runtime_error("Can't create temporary file");
  }
  close(tempFileDescriptor);
  std::remove(buffer);
  return std::string(buffer);
}();

template <class Key, class Value>
db_as_map::unordered_map<Key, Value>::unordered_map(const std::string& db_name)
    : db_name(db_name), db(nullptr) {
  sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
  sqlite3* db_ptr;
  if (sqlite3_open_v2(
          db_name.c_str(),
          &db_ptr,
          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
          nullptr) != SQLITE_OK) {
    throw std::runtime_error("Can't open database: " + db_name);
  }
  db = db_ptr;

  if (sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr) !=
      SQLITE_OK) {
    throw std::runtime_error("Can't set journal mode");
  }

  if (sqlite3_exec(db, "PRAGMA synchronous=OFF;", nullptr, nullptr, nullptr) !=
      SQLITE_OK) {
    throw std::runtime_error("Can't set synchronous mode");
  }

  while (sqlite3_table_column_metadata(
             db,
             nullptr,
             ("map" + std::to_string(_table_cnt)).c_str(),
             nullptr,
             nullptr,
             nullptr,
             nullptr,
             nullptr,
             nullptr) == SQLITE_OK) {
    ++_table_cnt;
  }
  table_name = "map" + std::to_string(_table_cnt++);
  create_table();
}

template <class Key, class Value>
db_as_map::unordered_map<Key, Value>::unordered_map()
    : unordered_map(default_db_name.c_str()) {}

template <class Key, class Value>
void db_as_map::unordered_map<Key, Value>::create_table() {
  char* err_msg = nullptr;

#define _SPECIAL_CASE_MACRO(_T, _S)                                            \
  if constexpr (std::is_same<Value, _T>::value) {                              \
    if (sqlite3_exec(                                                          \
            db,                                                                \
            ("CREATE TABLE IF NOT EXISTS " + table_name +                      \
             " (key TEXT PRIMARY KEY, "                                        \
             "value " +                                                        \
             std::string(_S) + ");")                                           \
                .c_str(),                                                      \
            nullptr,                                                           \
            nullptr,                                                           \
            &err_msg) != SQLITE_OK) {                                          \
      throw std::runtime_error("Can't create table: " + std::string(err_msg)); \
    }                                                                          \
  }

  _SPECIAL_CASE_MACRO(bool, "INTEGER");
  _SPECIAL_CASE_MACRO(char, "INTEGER");
  _SPECIAL_CASE_MACRO(signed char, "INTEGER");
  _SPECIAL_CASE_MACRO(unsigned char, "INTEGER");
  _SPECIAL_CASE_MACRO(wchar_t, "INTEGER");
  _SPECIAL_CASE_MACRO(char16_t, "INTEGER");
  _SPECIAL_CASE_MACRO(char32_t, "INTEGER");
  _SPECIAL_CASE_MACRO(short, "INTEGER");
  _SPECIAL_CASE_MACRO(int, "INTEGER");
  _SPECIAL_CASE_MACRO(long, "INTEGER");
  _SPECIAL_CASE_MACRO(long long, "INTEGER");
  _SPECIAL_CASE_MACRO(unsigned short, "INTEGER");
  _SPECIAL_CASE_MACRO(unsigned int, "INTEGER");
  _SPECIAL_CASE_MACRO(unsigned long, "INTEGER");
  _SPECIAL_CASE_MACRO(unsigned long long, "INTEGER");
  _SPECIAL_CASE_MACRO(float, "REAL");
  _SPECIAL_CASE_MACRO(double, "REAL");
  _SPECIAL_CASE_MACRO(long double, "REAL");
  _SPECIAL_CASE_MACRO(std::string, "TEXT");
  _SPECIAL_CASE_MACRO(std::wstring, "TEXT");
  _SPECIAL_CASE_MACRO(std::u16string, "TEXT");
  _SPECIAL_CASE_MACRO(std::u32string, "TEXT");
#undef _SPECIAL_CASE_MACRO
  if (sqlite3_exec(
          db,
          ("CREATE TABLE IF NOT EXISTS " + table_name +
           " (key TEXT PRIMARY KEY, "
           "value BLOB);")
              .c_str(),
          nullptr,
          nullptr,
          &err_msg) != SQLITE_OK) {
    throw std::runtime_error("Can't create table: " + std::string(err_msg));
  }
}

template <class Key, class Value>
void db_as_map::unordered_map<Key, Value>::sqlite3_bind_value(
    sqlite3_stmt* stmt,
    int idx,
    const Value& value) {
  if constexpr (std::is_same<Value, float>::value) {
    sqlite3_bind_double(stmt, idx, value);
  } else if constexpr (std::is_same<Value, double>::value) {
    sqlite3_bind_double(stmt, idx, value);
  } else if constexpr (
      std::is_integral<Value>::value || std::is_enum<Value>::value) {
    sqlite3_bind_int64(stmt, idx, value);
  } else if constexpr (std::is_same<Value, bool>::value) {
    sqlite3_bind_int(stmt, idx, value ? 1 : 0);
  } else if constexpr (
      std::is_same<Value, std::string>::value ||
      std::is_same<Value, std::wstring>::value ||
      std::is_same<Value, std::u16string>::value ||
      std::is_same<Value, std::u32string>::value) {
    sqlite3_bind_text(stmt, idx, value.c_str(), value.size(), SQLITE_TRANSIENT);
  } else {
    sqlite3_bind_blob(
        stmt,
        idx,
        Serialize<Value>::serialize(value),
        Serialize<Value>::size(value),
        SQLITE_TRANSIENT);
  }
}

template <class Key, class Value>
void db_as_map::unordered_map<Key, Value>::sqlite3_bind_key(
    sqlite3_stmt* stmt,
    int idx,
    const Key& key) {
  std::string key_str;
  if constexpr (
      std::is_same<Key, std::string>::value ||
      std::is_same<Key, std::wstring>::value ||
      std::is_same<Key, std::u16string>::value ||
      std::is_same<Key, std::u32string>::value)
    key_str = key;
  else
    key_str = std::to_string(key);
  sqlite3_bind_text(
      stmt, idx, key_str.c_str(), key_str.size(), SQLITE_TRANSIENT);
}

template <class Key, class Value>
void db_as_map::unordered_map<Key, Value>::insert(
    const Key& key,
    const Value& value) {
  if (in(key)) {
    throw std::runtime_error("Key already exists");
  }
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(
          db,
          ("INSERT INTO " + table_name +
           " (key, value) VALUES "
           "(?, ?);")
              .c_str(),
          -1,
          &stmt,
          nullptr) != SQLITE_OK) {
    throw std::runtime_error("Can't prepare statement");
  }
  this->sqlite3_bind_key(stmt, 1, key);
  this->sqlite3_bind_value(stmt, 2, value);
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    throw std::runtime_error("Can't insert value");
  }
  sqlite3_finalize(stmt);
}

template <class Key, class Value>
bool db_as_map::unordered_map<Key, Value>::erase(const Key& key) {
  if (!in(key)) {
    return false;
  }
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(
          db,
          ("DELETE FROM " + table_name + " WHERE key = ?;").c_str(),
          -1,
          &stmt,
          nullptr) != SQLITE_OK) {
    throw std::runtime_error("Can't prepare statement");
  }
  this->sqlite3_bind_key(stmt, 1, key);
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    throw std::runtime_error("Can't delete value");
  }
  sqlite3_finalize(stmt);
  return true;
}

template <class Key, class Value>
bool db_as_map::unordered_map<Key, Value>::in(const Key& key) {
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(
          db,
          ("SELECT value FROM " + table_name + " WHERE key = ?;").c_str(),
          -1,
          &stmt,
          nullptr) != SQLITE_OK) {
    throw std::runtime_error("Can't prepare statement");
  }
  this->sqlite3_bind_key(stmt, 1, key);
  auto ret = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return ret == SQLITE_ROW;
}

template <class Key, class Value>
Value db_as_map::unordered_map<Key, Value>::at(const Key& key) {
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(
          db,
          ("SELECT value FROM " + table_name + " WHERE key = ?;").c_str(),
          -1,
          &stmt,
          nullptr) != SQLITE_OK) {
    throw std::runtime_error("Can't prepare statement");
  }
  this->sqlite3_bind_key(stmt, 1, key);
  if (sqlite3_step(stmt) != SQLITE_ROW) {
    throw std::runtime_error("Can't find value");
  }
  if constexpr (std::is_same<Value, float>::value) {
    auto value = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    return value;
  } else if constexpr (std::is_same<Value, double>::value) {
    auto value = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    return value;
  } else if constexpr (
      std::is_integral<Value>::value || std::is_enum<Value>::value) {
    auto value = sqlite3_column_int64(stmt, 0);
    sqlite3_finalize(stmt);
    return value;
  } else if constexpr (std::is_same<Value, bool>::value) {
    auto value = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return value;
  } else if constexpr (
      std::is_same<Value, std::string>::value ||
      std::is_same<Value, std::wstring>::value ||
      std::is_same<Value, std::u16string>::value ||
      std::is_same<Value, std::u32string>::value) {
    auto value = std::string(
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
        sqlite3_column_bytes(stmt, 0));
    sqlite3_finalize(stmt);
    return value;
  } else {
    auto value = Serialize<Value>::deserialize(
        sqlite3_column_blob(stmt, 0), sqlite3_column_bytes(stmt, 0));
    sqlite3_finalize(stmt);
    return value;
  }
}

template <class Key, class Value>
void db_as_map::unordered_map<Key, Value>::replace(
    const Key& key,
    const Value& value) {
  if (!in(key)) {
    throw std::runtime_error("Key doesn't exist");
  }
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(
          db,
          ("UPDATE " + table_name + " SET value = ? WHERE key = ?;").c_str(),
          -1,
          &stmt,
          nullptr) != SQLITE_OK) {
    throw std::runtime_error("Can't prepare statement");
  }
  this->sqlite3_bind_key(stmt, 2, key);
  this->sqlite3_bind_value(stmt, 1, value);
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    throw std::runtime_error("Can't update value");
  }
  sqlite3_finalize(stmt);
}

template <class Key, class Value>
void db_as_map::unordered_map<Key, Value>::set(
    const Key& key,
    const Value& value) {
  if (in(key)) {
    replace(key, value);
  } else {
    insert(key, value);
  }
}

template <class Key, class Value>
db_as_map::unordered_map<Key, Value>::~unordered_map() {
  // clear the table
  // sqlite3_exec(
  //     db,
  //     ("DROP TABLE IF EXISTS " + table_name + ";").c_str(),
  //     nullptr,
  //     nullptr,
  //     nullptr);

  sqlite3_stmt* stmt;
  const char* sql =
      "SELECT COUNT(*) AS table_count FROM sqlite_master WHERE type='table';";
  sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

  size_t table_count = __SIZE_MAX__;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    table_count = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);

  sqlite3_close(db);
  // if (table_count == 0 && db_name != ":memory:") {
  // std::remove(db_name.c_str());
  // }
}

template <class Key, class Value>
void db_as_map::unordered_map<Key, Value>::sync() {
  // sqlite3_wal_checkpoint_v2(
  //     db, nullptr, SQLITE_CHECKPOINT_PASSIVE, nullptr, nullptr);
}
