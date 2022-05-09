#ifndef TILEDB_LABEL_QUERY_H
#define TILEDB_LABEL_QUERY_H

#include <string>
#include <unordered_map>
#include <vector>
#include "tiledb/common/common.h"
#include "tiledb/sm/filesystem/uri.h"
#include "tiledb/sm/label_query/axis_query.h"
#include "tiledb/sm/query/query.h"

using namespace tiledb::common;

namespace tiledb::sm {

class Array;
class StorageManager;
class Subarrray;
class Query;

enum class QueryStatus : uint8_t;
enum class QueryType : uint8_t;

class LabelledQuery {
 public:
  /** Default constructor is not C.41 compliant. */
  LabelledQuery() = delete;

  /**
   * Constructor.
   *
   * TODO: Add docs for params
   **/
  LabelledQuery(
      StorageManager* storage_manager,
      Array* array,
      URI fragment_uri = URI(""));

  /** Destructor. */
  ~LabelledQuery() = default;

  /** Disable copy and move. */
  DISABLE_COPY_AND_COPY_ASSIGN(LabelledQuery);
  DISABLE_MOVE_AND_MOVE_ASSIGN(LabelledQuery);

  /** TODO Add docs. */
  void add_external_dimension_label(
      const unsigned dim_idx,
      const LabelOrderType order_type,
      const std::string& internal_label_name,
      const std::string& internal_index_name,
      Array* array);

  //   /**
  //    * Adds a range to the (read/write) query on the input dimension by
  //    index,
  //    * in the form of (start, end, stride).
  //    * The range components must be of the same type as the domain type of
  //    the
  //    * underlying array.
  //    */
  //   Status add_range(
  //       unsigned dim_idx, const void* start, const void* end, const void*
  //       stride);
  //
  //   /**
  //    * Adds a variable-sized range to the (read/write) query on the input
  //    * dimension by index, in the form of (start, end).
  //    */
  //   Status add_range_var(
  //       unsigned dim_idx,
  //       const void* start,
  //       uint64_t start_size,
  //       const void* end,
  //       uint64_t end_size);
  //
  //   /**
  //    * Marks a query that has not yet been started as failed. This should not
  //    be
  //    * called asynchronously to cancel an in-progress query; for that use the
  //    * parent StorageManager's cancellation mechanism.
  //    * @return Status
  //    */
  //   Status cancel();
  //
  //   /**
  //    * Finalizes the query, flushing all internal state. Applicable only to
  //    global
  //    * layout writes. It has no effect for any other query type.
  //    */
  //   Status finalize();
  //
  //   /**
  //    * Gets the estimated result size (in bytes) for the input fixed-sized
  //    * attribute/dimension.
  //    */
  //   Status get_est_result_size(const char* name, uint64_t* size);
  //
  //   /**
  //    * Gets the estimated result size (in bytes) for the input var-sized
  //    * attribute/dimension.
  //    */
  //   Status get_est_result_size(
  //       const char* name, uint64_t* size_off, uint64_t* size_val);
  //
  //   /**
  //    * Gets the estimated result size (in bytes) for the input fixed-sized,
  //    * nullable attribute.
  //    */
  //   Status get_est_result_size_nullable(
  //       const char* name, uint64_t* size_val, uint64_t* size_validity);
  //
  //   /**
  //    * Gets the estimated result size (in bytes) for the input var-sized,
  //    * nullable attribute.
  //    */
  //   Status get_est_result_size_nullable(
  //       const char* name,
  //       uint64_t* size_off,
  //       uint64_t* size_val,
  //       uint64_t* size_validity);
  //
  //   /**
  //    * Retrieves the data buffer of a fixed/var-sized attribute/dimension.
  //    *
  //    * @param name The buffer attribute/dimension name. An empty string means
  //    *     the special default attribute/dimension.
  //    * @param buffer The buffer to be retrieved.
  //    * @param buffer_size A pointer to the buffer size to be retrieved.
  //    * @return Status
  //    */
  //   Status get_data_buffer(
  //       const char* name, void** buffer, uint64_t** buffer_size) const;
  //
  //   /**
  //    * Retrieves the offset buffer for a var-sized attribute/dimension.
  //    *
  //    * @param name The buffer attribute/dimension name. An empty string means
  //    * the special default attribute/dimension.
  //    * @param buffer_off The offsets buffer to be retrieved. This buffer
  //    holds
  //    * the starting offsets of each cell value in the data buffer.
  //    * @param buffer_off_size A pointer to the buffer size to be retrieved.
  //    * @return Status
  //    */
  //   Status get_offsets_buffer(
  //       const char* name,
  //       uint64_t** buffer_off,
  //       uint64_t** buffer_off_size) const;
  //
  //   /**
  //    * Retrieves the validity buffer for a nullable attribute/dimension.
  //    *
  //    * @param name The buffer attribute/dimension name. An empty string means
  //    * the special default attribute/dimension.
  //    * @param buffer_validity_bytemap The buffer that either have the
  //    validity
  //    * bytemap associated with the input data to be written, or will hold the
  //    * validity bytemap to be read.
  //    * @param buffer_validity_bytemap_size In the case of writes, this is the
  //    size
  //    * of `buffer_validity_bytemap` in bytes. In the case of reads, this
  //    initially
  //    * contains the allocated size of `buffer_validity_bytemap`, but after
  //    the
  //    * termination of the query it will contain the size of the useful (read)
  //    * data in `buffer_validity_bytemap`.
  //    * @return Status
  //    */
  //   Status get_validity_buffer(
  //       const char* name,
  //       uint8_t** buffer_validity_bytemap,
  //       uint64_t** buffer_validity_bytemap_size) const;
  //
  //   /**
  //    * Returns `true` if the query has results. Applicable only to read
  //    * queries (it returns `false` for write queries).
  //    */
  //   bool has_results() const;
  //
  //   /** Initializes the query. */
  //   Status init();
  //
  //   /**
  //    * Sets the data for a fixed/var-sized attribute/dimension.
  //    *
  //    * @param name The attribute/dimension to set the buffer for.
  //    * @param buffer The buffer that will hold the data to be read.
  //    * @param buffer_size This initially contains the allocated
  //    *     size of `buffer`, but after the termination of the function
  //    *     it will contain the size of the useful (read) data in `buffer`.
  //    * @param check_null_buffers If true (default), null buffers are not
  //    * allowed.
  //    * @return Status
  //    */
  //   Status set_data_buffer(
  //       const std::string& name,
  //       void* const buffer,
  //       uint64_t* const buffer_size,
  //       const bool check_null_buffers = true);
  //
  //   /**
  //    * Sets the offset buffer for a var-sized attribute/dimension.
  //    *
  //    * @param name The attribute/dimension to set the buffer for.
  //    * @param buffer_offsets The buffer that will hold the data to be read.
  //    *     This buffer holds the starting offsets of each cell value in
  //    *     `buffer_val`.
  //    * @param buffer_offsets_size This initially contains
  //    *     the allocated size of `buffer_off`, but after the termination of
  //    the
  //    *     function it will contain the size of the useful (read) data in
  //    *     `buffer_off`.
  //    * @param check_null_buffers If true (default), null buffers are not
  //    * allowed.
  //    * @return Status
  //    */
  //   Status set_offsets_buffer(
  //       const std::string& name,
  //       uint64_t* const buffer_offsets,
  //       uint64_t* const buffer_offsets_size,
  //       const bool check_null_buffers = true);
  //
  //   /**
  //    * Sets the offset buffer for a var-sized attribute/dimension.
  //    *
  //    * @param name The attribute/dimension to set the buffer for.
  //    * @param buffer_offsets The buffer that will hold the data to be read.
  //    *     This buffer holds the starting offsets of each cell value in
  //    *     `buffer_val`.
  //    * @param buffer_offsets_size This initially contains
  //    *     the allocated size of `buffer_off`, but after the termination of
  //    the
  //    *     function it will contain the size of the useful (read) data in
  //    *     `buffer_off`.
  //    * @param check_null_buffers If true (default), null buffers are not
  //    * allowed.
  //    * @return Status
  //    */
  //   Status set_offsets_buffer(
  //       const std::string& name,
  //       uint64_t* const buffer_offsets,
  //       uint64_t* const buffer_offsets_size,
  //       const bool check_null_buffers = true);
  //
  //   /**
  //    * Sets the validity buffer for nullable attribute/dimension.
  //    *
  //    * @param name The attribute/dimension to set the buffer for.
  //    * @param buffer_validity_bytemap The buffer that either have the
  //    validity
  //    * bytemap associated with the input data to be written, or will hold the
  //    * validity bytemap to be read.
  //    * @param buffer_validity_bytemap_size In the case of writes, this is the
  //    size
  //    * of `buffer_validity_bytemap` in bytes. In the case of reads, this
  //    initially
  //    * contains the allocated size of `buffer_validity_bytemap`, but after
  //    the
  //    * termination of the query it will contain the size of the useful (read)
  //    * data in `buffer_validity_bytemap`.
  //    * @param check_null_buffers If true (default), null buffers are not
  //    * allowed.
  //    * @return Status
  //    */
  //   Status set_validity_buffer(
  //       const std::string& name,
  //       uint8_t* const buffer_validity_bytemap,
  //       uint64_t* const buffer_validity_bytemap_size,
  //       const bool check_null_buffers = true);
  //
  //   /**
  //    * Sets the cell layout of the query.
  //    */
  //   Status set_layout(Layout layout);
  //
  //   /**
  //    * Sets the query subarray. If it is null, then the subarray will be set
  //    to
  //    * the entire domain.
  //    *
  //    * @param subarray The subarray to be set.
  //    * @return Status
  //    *
  //    * @note Setting a subarray for sparse arrays, or for dense arrays
  //    *     when performing unordered (sparse) writes, has no effect
  //    *     (will be ingnored).
  //    */
  //   Status set_subarray(const void* subarray);
  //
  //   /** Submits the query to the storage manager. */
  //   Status submit();
  //
  //   /**
  //    * Submits the query to the storage manager. The query will be
  //    * processed asynchronously (i.e., in a non-blocking manner).
  //    * Once the query is completed, the input callback function will
  //    * be executed using the input callback data.
  //    */
  //   Status submit_async(std::function<void(void*)> callback, void*
  //   callback_data);

 private:
  StorageManager* storage_manager_;
  Query query_;
  unsigned dim_num_;
  std::vector<shared_ptr<AxisQuery>> dimension_label_queries_;
  std::vector<std::vector<AxisQuery>> extra_label_queries_;
  std::vector<std::vector<AxisQuery>> extra_dim_queries_;

  /** The type of the query. */
  QueryType type_;
};

}  // namespace tiledb::sm

#endif
