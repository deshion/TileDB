#ifndef TILEDB_AXIS_QUERY_H
#define TILEDB_AXIS_QUERY_H

#include <string>
#include <unordered_map>
#include <vector>
#include "tiledb/sm/label_query/axis_subarray.h"
#include "tiledb/sm/query/query.h"

namespace tiledb::sm {

class Array;
class Query;
enum class QueryType : uint8_t;

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

  // /**
  //  * Retrieves the data buffer of a fixed/var-sized attribute/dimension.
  //  *
  //  * @param name The buffer attribute/dimension name. An empty string means
  //  *     the special default attribute/dimension.
  //  * @param buffer The buffer to be retrieved.
  //  * @param buffer_size A pointer to the buffer size to be retrieved.
  //  * @return Status
  //  */
  // virtual Status get_data_buffer(
  //     const char* name, void** buffer, uint64_t** buffer_size) const = 0;

  /**
   * Returns `true` if the query has results. Applicable only to read
   * queries (it returns `false` for write queries).
   */
  // virtual bool has_results() const = 0;

  /** Initializes the query. */
  virtual Status init() = 0;

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

class UnorderedAxisQuery : public AxisQuery {
 public:
  UnorderedAxisQuery() = delete;
  UnorderedAxisQuery(
      const AxisSubarray& subarray, StorageManager* storage_manager);
  /** Disable copy and move. */
  DISABLE_COPY_AND_COPY_ASSIGN(UnorderedAxisQuery);
  DISABLE_MOVE_AND_MOVE_ASSIGN(UnorderedAxisQuery);

  Status cancel() override;

  Status finalize() override;

  Status init() override;

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
  Query query_;
  AxisSubarray subarray_;  // TODO Change this. Maybe pointer?
};

}  // namespace tiledb::sm

#endif
