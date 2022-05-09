#ifndef TILEDB_AXIS_QUERY_H
#define TILEDB_AXIS_QUERY_H

#include <string>
#include <unordered_map>
#include <vector>
#include "tiledb/sm/query/query.h"

namespace tiledb::sm {

class Array;
class Query;
enum class QueryType : uint8_t;

enum class LabelOrderType : uint8_t { UNORDERED, FORWARD, REVERSE };

class AxisQuery {
 public:
  AxisQuery(
      const QueryType type, StorageManager* storage_manager, Array* array);
  virtual ~AxisQuery() = default;

 protected:
 private:
  Query query_;
};

class ForwardAxisQuery : public AxisQuery {
 public:
  ForwardAxisQuery() = delete;

  ForwardAxisQuery(
      const std::string& label_name,
      const std::string& index_name,
      StorageManager* storage_manager,
      Array* array);

 private:
  std::string index_name_;
};

class ReverseAxisQuery : public AxisQuery {
 public:
  ReverseAxisQuery() = delete;
  ReverseAxisQuery(
      const std::string& label_name,
      const std::string& index_name,
      StorageManager* storage_manager,
      Array* array);

 private:
  std::string index_name_;
};

class UnorderedAxisQuery : public AxisQuery {
 public:
  UnorderedAxisQuery() = delete;
  UnorderedAxisQuery(
      const std::string& label_name,
      const std::string& index_name,
      StorageManager* storage_manager,
      Array* array);

 private:
  std::string index_name_;
};

class DataAxisQuery : public AxisQuery {
 public:
  DataAxisQuery() = delete;
  DataAxisQuery(
      const std::string& label_name,
      const std::string& index_name,
      StorageManager* storage_manager,
      Array* array);

 private:
  std::string label_name_;
};

}  // namespace tiledb::sm

#endif
