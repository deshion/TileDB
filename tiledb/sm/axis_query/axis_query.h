#ifndef TILEDB_AXIS_QUERY_H
#define TILEDB_AXIS_QUERY_H

#include <string>
#include "tiledb/sm/axis/axis.h"
#include "tiledb/sm/query/query.h"

namespace tiledb::sm {

class Array;
class Axis;
class Query;
enum class QueryType : uint8_t;

/** Return a Status_AxisQueryError error class Status with a given
 * message. Note: currently set to return Query error. **/
inline Status Status_AxisQueryError(const std::string& msg) {
  return {"[TileDB::Query] Error", msg};
};

class AxisQuery {
 public:
  virtual ~AxisQuery() = default;

  /**
   * Marks a query that has not yet been started as failed. This should not be
   * called asynchronously to cancel an in-progress query; for that use the
   * parent StorageManager's cancellation mechanism.
   * @return Status
   */
  virtual Status cancel() = 0;

  virtual Status finalize() = 0;

  /**
   * Retrieves the data buffer of a fixed/var-sized attribute/dimension.
   *
   * @param name The buffer attribute/dimension name. An empty string means
   *     the special default attribute/dimension.
   * @param buffer The buffer to be retrieved.
   * @param buffer_size A pointer to the buffer size to be retrieved.
   * @return Status
   */
  virtual tuple<Status, void*, uint64_t> get_index_point_ranges() const = 0;

  /**
   * Returns `true` if the query has results. Applicable only to read
   * queries (it returns `false` for write queries).
   */
  // virtual bool has_results() const = 0;

  /** Initializes the query. */
  virtual Status init() = 0;

  /** TODO */
  virtual Status add_range(
      const void* start, const void* end, const void* stride) = 0;

  /** TODO */
  virtual Status add_range_var(
      const void* start,
      uint64_t start_size,
      const void* end,
      uint64_t end_size) = 0;

  /**
   * Sets the data for an axis where the indix values are fixed sized.
   *
   * @param buffer The buffer that will hold the data to be read.
   * @param buffer_size This initially contains the allocated
   *     size of `buffer`, but after the termination of the function
   *     it will contain the size of the useful (read) data in `buffer`.
   * @param check_null_buffers If true, null buffers are not allowed.
   * @return Status
   */
  virtual Status set_index_data_buffer(
      void* const buffer,
      uint64_t* const buffer_size,
      const bool check_null_buffers) = 0;

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
  virtual QueryStatus status() const = 0;

  /** Submits the query to the storage manager. */
  virtual Status submit() = 0;
};

class OrderedAxisQuery : public AxisQuery {
 public:
  OrderedAxisQuery() = delete;

  OrderedAxisQuery(
      shared_ptr<Axis> axis,
      StorageManager* storage_manager,
      const std::string& label_name);

  /** Disable copy and move. */
  DISABLE_COPY_AND_COPY_ASSIGN(OrderedAxisQuery);
  DISABLE_MOVE_AND_MOVE_ASSIGN(OrderedAxisQuery);

  Status cancel() override;

  Status finalize() override;

  tuple<Status, void*, uint64_t> get_index_point_ranges() const override;

  Status init() override;

  Status add_range(
      const void* start, const void* end, const void* stride) override;

  Status add_range_var(
      const void* start,
      uint64_t start_size,
      const void* end,
      uint64_t end_size) override;

  Status set_index_data_buffer(
      void* const buffer,
      uint64_t* const buffer_size,
      const bool check_null_buffers) override;

  Status set_label_data_buffer(
      void* const buffer,
      uint64_t* const buffer_size,
      const bool check_null_buffers) override;

  QueryStatus status() const override;

  Status submit() override;

 private:
  shared_ptr<Axis> axis_;
  StorageManager* storage_manager_;
  std::string label_name_;
  std::string internal_label_name_;
  std::string internal_index_name_;
  tdb_unique_ptr<Query> range_query_;
};

}  // namespace tiledb::sm

#endif
