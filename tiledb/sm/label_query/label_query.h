#ifndef TILEDB_LABEL_QUERY_H
#define TILEDB_LABEL_QUERY_H

#include <string>
#include <unordered_map>
#include <vector>
#include "tiledb/common/common.h"
#include "tiledb/common/logger_public.h"
#include "tiledb/sm/filesystem/uri.h"
#include "tiledb/sm/label_query/axis_query.h"
#include "tiledb/sm/query/query.h"

using namespace tiledb::common;

namespace tiledb::sm {

class Array;
class StorageManager;
class Subarrray;
class LabelledSubarray;
class Query;

enum class QueryStatus : uint8_t;
enum class QueryType : uint8_t;

/** Return a Status_LabelledQuery error class Status with a given message **/
inline Status Status_LabelledQueryError(const std::string& msg) {
  return {"[TileDB::LabelledQuery] Error", msg};
}

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
      const LabelledSubarray& subarray,
      StorageManager* storage_manager,
      Array* array,
      URI fragment_uri = URI(""));

  /** Destructor. */
  ~LabelledQuery() = default;

  /** Disable copy and move. */
  DISABLE_COPY_AND_COPY_ASSIGN(LabelledQuery);
  DISABLE_MOVE_AND_MOVE_ASSIGN(LabelledQuery);

  /**
   * Marks a query that has not yet been started as failed. This should not be
   * called asynchronously to cancel an in-progress query; for that use the
   * parent StorageManager's cancellation mechanism.
   * @return Status
   */
  Status cancel();

  /**
   * Finalizes the query, flushing all internal state. Applicable only to global
   * layout writes. It has no effect for any other query type.
   */
  Status finalize();

  /**
   * Finalizes the label queries, flushing all internal state. Applicable only
   * to global layout writes. It has no effect for any other query type.
   */
  Status finalize_labels();

  /**
   * Gets the estimated result size (in bytes) for the input fixed-sized
   * attribute/dimension.
   */
  // Status get_est_result_size(const char* name, uint64_t* size);

  /**
   * Gets the estimated result size (in bytes) for the input var-sized
   * attribute/dimension.
   */
  // Status get_est_result_size(
  //     const char* name, uint64_t* size_off, uint64_t* size_val);

  /**
   * Gets the estimated result size (in bytes) for the input fixed-sized,
   * nullable attribute.
   */
  // Status get_est_result_size_nullable(
  //     const char* name, uint64_t* size_val, uint64_t* size_validity);

  /**
   * Gets the estimated result size (in bytes) for the input var-sized,
   * nullable attribute.
   */
  // Status get_est_result_size_nullable(
  //     const char* name,
  //     uint64_t* size_off,
  //     uint64_t* size_val,
  //     uint64_t* size_validity);

  /**
   * Retrieves the data buffer of a fixed/var-sized attribute/dimension.
   *
   * @param name The buffer attribute/dimension name. An empty string means
   *     the special default attribute/dimension.
   * @param buffer The buffer to be retrieved.
   * @param buffer_size A pointer to the buffer size to be retrieved.
   * @return Status
   */
  // Status get_data_buffer(
  //     const char* name, void** buffer, uint64_t** buffer_size) const;

  /**
   * Retrieves the offset buffer for a var-sized attribute/dimension.
   *
   * @param name The buffer attribute/dimension name. An empty string means
   * the special default attribute/dimension.
   * @param buffer_off The offsets buffer to be retrieved. This buffer holds
   * the starting offsets of each cell value in the data buffer.
   * @param buffer_off_size A pointer to the buffer size to be retrieved.
   * @return Status
   */
  // Status get_offsets_buffer(
  //     const char* name,
  //     uint64_t** buffer_off,
  //     uint64_t** buffer_off_size) const;

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
  // Status get_validity_buffer(
  //     const char* name,
  //     uint8_t** buffer_validity_bytemap,
  //     uint64_t** buffer_validity_bytemap_size) const;

  /**
   * Returns `true` if the query has results. Applicable only to read
   * queries (it returns `false` for write queries).
   */
  // bool has_results() const;

  /** Initializes the query. */
  Status init();

  /** Initializes the label queries. */
  Status init_labels();

  /** TODO Add docs. */
  bool label_queries_completed();

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
   * Sets the data for a fixed/var-sized dimension label.
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
  Status set_label_data_buffer(
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
   *     the allocated size of `buffer_off`, but after the termination of
   the
   *     function it will contain the size of the useful (read) data in
   *     `buffer_off`.
   * @param check_null_buffers If true (default), null buffers are not
   * allowed.
   * @return Status
   */
  // Status set_offsets_buffer(
  //     const std::string& name,
  //     uint64_t* const buffer_offsets,
  //     uint64_t* const buffer_offsets_size,
  //     const bool check_null_buffers = true);

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
  //  Status set_validity_buffer(
  //      const std::string& name,
  //      uint8_t* const buffer_validity_bytemap,
  //      uint64_t* const buffer_validity_bytemap_size,
  //      const bool check_null_buffers = true);

  /** Returns the query status. */
  QueryStatus status() const;

  /** Submits the label queries to the storage manager. */
  // Status submit_labels();

  /** Submits the query to the storage manager. */
  // Status submit();

  /**
   * Submits the query to the storage manager. The query will be
   * processed asynchronously (i.e., in a non-blocking manner).
   * Once the query is completed, the input callback function will
   * be executed using the input callback data.
   */
  //  Status submit_async(std::function<void(void*)> callback, void*
  //  callback_data);

 private:
  /** TODO: Add docs */
  [[maybe_unused]] StorageManager* storage_manager_;

  /** TODO: Add docs */
  Query query_;

  /** TODO: Add docs */
  unsigned dim_num_;

  /** TODO: Add docs */
  std::vector<shared_ptr<AxisQuery>> dimension_label_queries_;

  /** Labels by name. */
  std::unordered_map<std::string, AxisQuery*> label_map_;

  /** The type of the query. */
  QueryType type_;
};

}  // namespace tiledb::sm

#endif
