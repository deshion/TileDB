#ifndef TILEDB_RANGE_QUERY_H
#define TILEDB_RANGE_QUERY_H

#include <string>
#include "tiledb/sm/axis/axis.h"
#include "tiledb/sm/enums/query_status.h"
#include "tiledb/sm/query/query.h"

namespace tiledb::sm {

class Array;
class Axis;
class Query;

/** Return a Status_AxisQueryError error class Status with a given
 * message. Note: currently set to return Query error. **/
inline Status Status_RangeQueryError(const std::string& msg) {
  return {"[TileDB::Query] Error", msg};
};

/**
 * This class should be considered deprecated at creation. This RangeQuery
 * should be replaced with a range strategy that can handle multiple ranges
 * inside a single query.
 */
class RangeQuery {
 public:
  RangeQuery() = delete;

  RangeQuery(
      Axis* axis,
      StorageManager* storage_manager,
      const void* start,
      const void* end);

  /** Disable copy and move. */
  DISABLE_COPY_AND_COPY_ASSIGN(RangeQuery);
  DISABLE_MOVE_AND_MOVE_ASSIGN(RangeQuery);

  Status cancel();
  Status finalize();

  inline const Range& index_range() const {
    return computed_index_range_;
  }

  inline QueryStatus status() const {
    return status_;
  }
  Status submit();

 private:
  LabelOrder order_;
  uint64_t label_data_size_;
  uint64_t index_data_size_;
  Range input_label_range_;
  Range computed_label_range_;
  Range computed_index_range_;
  Query lower_bound_query_;
  Query upper_bound_query_;
  QueryStatus status_{QueryStatus::UNINITIALIZED};
};

}  // namespace tiledb::sm

#endif
