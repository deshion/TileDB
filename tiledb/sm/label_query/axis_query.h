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

  //  /**
  //   * Gets the estimated result size (in bytes) for the input fixed-sized
  //   * attribute/dimension.
  //   */
  //  virtual Status get_est_result_size(const char* name, uint64_t* size) = 0;

  //  /**
  //   * Gets the estimated result size (in bytes) for the input var-sized
  //   * attribute/dimension.
  //   */
  //  virtual Status get_est_result_size(
  //      const char* name, uint64_t* size_off, uint64_t* size_val) = 0;

  //  /**
  //   * Gets the estimated result size (in bytes) for the input fixed-sized,
  //   * nullable attribute.
  //   */
  //  virtual Status get_est_result_size_nullable(
  //      const char* name, uint64_t* size_val, uint64_t* size_validity) = 0;

  // /**
  //  * Gets the estimated result size (in bytes) for the input var-sized,
  //  * nullable attribute.
  //  */
  // virtual Status get_est_result_size_nullable(
  //     const char* name,
  //     uint64_t* size_off,
  //     uint64_t* size_val,
  //     uint64_t* size_validity) = 0;

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

  // /**
  //  * Retrieves the offset buffer for a var-sized attribute/dimension.
  //  *
  //  * @param name The buffer attribute/dimension name. An empty string means
  //  * the special default attribute/dimension.
  //  * @param buffer_off The offsets buffer to be retrieved. This buffer
  //  holds
  //  * the starting offsets of each cell value in the data buffer.
  //  * @param buffer_off_size A pointer to the buffer size to be retrieved.
  //  * @return Status
  //  */
  // virtual Status get_offsets_buffer(
  //     const char* name,
  //     uint64_t** buffer_off,
  //     uint64_t** buffer_off_size) const = 0;

  /**
   * Retrieves the validity buffer for a nullable attribute/dimension.
   *
   * @param name The buffer attribute/dimension name. An empty string means
   * the special default attribute/dimension.
   * @param buffer_validity_bytemap The buffer that either have the
   validity
   * bytemap associated with the input data to be written, or will hold the
   * validity bytemap to be read.
   * @param buffer_validity_bytemap_size In the case of writes, this is the
   size
   * of `buffer_validity_bytemap` in bytes. In the case of reads, this
   initially
   * contains the allocated size of `buffer_validity_bytemap`, but after
   the
   * termination of the query it will contain the size of the useful (read)
   * data in `buffer_validity_bytemap`.
   * @return Status
   */
  virtual Status get_validity_buffer(
      const char* name,
      uint8_t** buffer_validity_bytemap,
      uint64_t** buffer_validity_bytemap_size) const = 0;

  /**
   * Returns `true` if the query has results. Applicable only to read
   * queries (it returns `false` for write queries).
   */
  virtual bool has_results() const = 0;

  /** Initializes the query. */
  virtual Status init() const = 0;

  /**
   * Sets the data for a fixed/var-sized attribute/dimension.
   *
   * @param name The attribute/dimension to set the buffer for.
   * @param buffer The buffer that will hold the data to be read.
   * @param buffer_size This initially contains the allocated
   *     size of `buffer`, but after the termination of the function
   *     it will contain the size of the useful (read) data in `buffer`.
   * @param check_null_buffers If true (default), null buffers are not
   * allowed.
   * @return Status
   */
  virtual Status set_data_buffer(
      const std::string& name,
      void* const buffer,
      uint64_t* const buffer_size,
      const bool check_null_buffers = true) = 0;

  /**
   * Sets the offset buffer for a var-sized attribute/dimension.
   *
   * @param name The attribute/dimension to set the buffer for.
   * @param buffer_offsets The buffer that will hold the data to be read.
   *     This buffer holds the starting offsets of each cell value in
   *     `buffer_val`.
   * @param buffer_offsets_size This initially contains
   *     the allocated size of `buffer_off`, but after the termination of
   the
   *     function it will contain the size of the useful (read) data in
   *     `buffer_off`.
   * @param check_null_buffers If true (default), null buffers are not
   * allowed.
   * @return Status
   */
  virtual Status set_offsets_buffer(
      const std::string& name,
      uint64_t* const buffer_offsets,
      uint64_t* const buffer_offsets_size,
      const bool check_null_buffers = true) = 0;

  /**
   * Sets the validity buffer for nullable attribute/dimension.
   *
   * @param name The attribute/dimension to set the buffer for.
   * @param buffer_validity_bytemap The buffer that either have the validity
   * bytemap associated with the input data to be written, or will hold the
   * validity bytemap to be read.
   * @param buffer_validity_bytemap_size In the case of writes, this is the size
   * of `buffer_validity_bytemap` in bytes. In the case of reads, this initially
   * contains the allocated size of `buffer_validity_bytemap`, but after the
   * termination of the query it will contain the size of the useful (read)
   * data in `buffer_validity_bytemap`.
   * @param check_null_buffers If true (default), null buffers are not
   * allowed.
   * @return Status
   */
  virtual Status set_validity_buffer(
      const std::string& name,
      uint8_t* const buffer_validity_bytemap,
      uint64_t* const buffer_validity_bytemap_size,
      const bool check_null_buffers = true) = 0;
};

class UnorderedAxisQuery : public AxisQuery {
 public:
  UnorderedAxisQuery() = delete;
  UnorderedAxisQuery(
      const std::string& label_name,
      const std::string& index_name,
      StorageManager* storage_manager,
      Array* array);

  Status cancel();

  /** Initializes the query. */
  Status init();

  /**
   * Sets the data for a fixed/var-sized attribute/dimension.
   *
   * @param name The attribute/dimension to set the buffer for.
   * @param buffer The buffer that will hold the data to be read.
   * @param buffer_size This initially contains the allocated
   *     size of `buffer`, but after the termination of the function
   *     it will contain the size of the useful (read) data in `buffer`.
   * @param check_null_buffers If true (default), null buffers are not
   * allowed.
   * @return Status
   */
  Status set_data_buffer(
      const std::string& name,
      void* const buffer,
      uint64_t* const buffer_size,
      const bool check_null_buffers = true);

  /**
   * Sets the offset buffer for a var-sized attribute/dimension.
   *
   * @param name The attribute/dimension to set the buffer for.
   * @param buffer_offsets The buffer that will hold the data to be read.
   *     This buffer holds the starting offsets of each cell value in
   *     `buffer_val`.
   * @param buffer_offsets_size This initially contains
   *     the allocated size of `buffer_off`, but after the termination of the
   *     function it will contain the size of the useful (read) data in
   *     `buffer_off`.
   * @param check_null_buffers If true (default), null buffers are not
   * allowed.
   * @return Status
   */
  Status set_offsets_buffer(
      const std::string& name,
      uint64_t* const buffer_offsets,
      uint64_t* const buffer_offsets_size,
      const bool check_null_buffers = true);

  /**
   * Sets the validity buffer for nullable attribute/dimension.
   *
   * @param name The attribute/dimension to set the buffer for.
   * @param buffer_validity_bytemap The buffer that either have the validity
   * bytemap associated with the input data to be written, or will hold the
   * validity bytemap to be read.
   * @param buffer_validity_bytemap_size In the case of writes, this is the size
   * of `buffer_validity_bytemap` in bytes. In the case of reads, this initially
   * contains the allocated size of `buffer_validity_bytemap`, but after the
   * termination of the query it will contain the size of the useful (read)
   * data in `buffer_validity_bytemap`.
   * @param check_null_buffers If true (default), null buffers are not
   * allowed.
   * @return Status
   */
  Status set_validity_buffer(
      const std::string& name,
      uint8_t* const buffer_validity_bytemap,
      uint64_t* const buffer_validity_bytemap_size,
      const bool check_null_buffers = true);

  /**
   * Sets the query subarray.
   *
   * @param subarray The subarray to be set.
   * @return Status
   */
  Status set_subarray(const Subarray& subarray);

  /** Returns the query status. */
  QueryStatus status() const;

  /** Submits the label queries to the storage manager. */
  Status submit_labels();

  /** Submits the query to the storage manager. */
  Status submit();

 private:
  Query query_;
  std::string index_name_;
};

}  // namespace tiledb::sm

#endif
