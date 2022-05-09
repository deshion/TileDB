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
  virtual ~AxisQuery() = default;
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
  Query query_;
  std::string index_name_;
};

}  // namespace tiledb::sm

#endif
