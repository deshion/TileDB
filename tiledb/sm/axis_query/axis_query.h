#ifndef TILEDB_AXIS_QUERY_H
#define TILEDB_AXIS_QUERY_H

#include <string>
#include "tiledb/sm/axis/axis.h"
#include "tiledb/sm/axis_query/range_query.h"
#include "tiledb/sm/enums/query_status.h"
#include "tiledb/sm/query/query.h"

namespace tiledb::sm {

class Array;
class Axis;
class Query;

/** Return a Status_AxisQueryError error class Status with a given
 * message. Note: currently set to return Query error. **/
inline Status Status_AxisQueryError(const std::string& msg) {
  return {"[TileDB::Query] Error", msg};
};

class AxisQuery {
 public:
  virtual ~AxisQuery() = default;

  /** TODO */
  virtual Status add_label_range(
      const void* start, const void* end, const void* stride) = 0;

  /** TODO */
  virtual Status add_label_range_var(
      const void* start,
      uint64_t start_size,
      const void* end,
      uint64_t end_size) = 0;

  /**
   * Marks a query that has not yet been started as failed. This should not be
   * called asynchronously to cancel an in-progress query; for that use the
   * parent StorageManager's cancellation mechanism.
   * @return Status
   */
  virtual Status cancel() = 0;

  virtual Status create_data_query() = 0;

  virtual Status finalize() = 0;

  virtual tuple<Status, Range> get_index_range() const = 0;

  /** TODO */
  virtual Status resolve_labels() = 0;

  /** TODO */
  virtual Status set_index_ranges(const std::vector<Range>& ranges) = 0;

  /**
   * Sets the data for an axis where the label values fixed sized.
   *
   * @param buffer The buffer that will hold the data to be read.
   * @param buffer_size This initially contains the allocated
   *     size of `buffer`, but after the termination of the function
   *     it will contain the size of the useful (read) data in `buffer`.
   * @param check_null_buffers If true, null buffers are not allowed.
   * @return Status
   */
  virtual Status set_label_data_buffer(
      void* const buffer,
      uint64_t* const buffer_size,
      const bool check_null_buffers) = 0;

  /** Returns the query status. */
  virtual QueryStatus status_data_query() const = 0;
  virtual QueryStatus status_resolve_labels() const = 0;

  /** Submits the query to the storage manager. */
  virtual Status submit_data_query() = 0;
};

class OrderedAxisQuery : public AxisQuery {
 public:
  OrderedAxisQuery() = delete;

  OrderedAxisQuery(shared_ptr<Axis> axis, StorageManager* storage_manager);

  /** Disable copy and move. */
  DISABLE_COPY_AND_COPY_ASSIGN(OrderedAxisQuery);
  DISABLE_MOVE_AND_MOVE_ASSIGN(OrderedAxisQuery);

  Status add_label_range(
      const void* start, const void* end, const void* stride) override;

  Status add_label_range_var(
      const void* start,
      uint64_t start_size,
      const void* end,
      uint64_t end_size) override;

  Status cancel() override;

  Status create_data_query() override;

  Status finalize() override;

  tuple<Status, Range> get_index_range() const override;

  Status resolve_labels() override;

  Status set_index_ranges(const std::vector<Range>& ranges) override;

  Status set_index_subarray(const void* subarray);

  Status set_label_data_buffer(
      void* const buffer,
      uint64_t* const buffer_size,
      const bool check_null_buffers) override;

  QueryStatus status_data_query() const override;

  QueryStatus status_resolve_labels() const override;

  Status submit_data_query() override;

 private:
  /** UID of the logger instance */
  inline static std::atomic<uint64_t> logger_id_ = 0;

  shared_ptr<Axis> axis_;

  StorageManager* storage_manager_;

  stats::Stats* stats_;

  shared_ptr<Logger> logger_;

  tdb_unique_ptr<RangeQuery> range_query_;

  tdb_unique_ptr<Query> labelled_array_query_;

  tdb_unique_ptr<Query> indexed_array_query_;
};

}  // namespace tiledb::sm

#endif
