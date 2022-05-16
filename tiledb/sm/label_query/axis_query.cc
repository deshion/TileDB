#include "tiledb/sm/label_query/axis_query.h"
#include "tiledb/sm/array/array.h"
#include "tiledb/sm/label_query/axis_subarray.h"
#include "tiledb/sm/query/query.h"
#include "tiledb/sm/storage_manager/storage_manager.h"

namespace tiledb::sm {

/**********************/
/* UnorderedAxisQuery */
/**********************/

UnorderedAxisQuery::UnorderedAxisQuery(
    const AxisSubarray& subarray, StorageManager* storage_manager)
    : query_{storage_manager, const_cast<Array*>(subarray.array())}
    , subarray_{subarray} {
  if (!subarray.array()->is_open())
    throw std::invalid_argument("Cannot query axis; array is not open.");
  QueryType actual_type;
  throw_if_not_ok(query_.array()->get_query_type(&actual_type));
  if (actual_type != QueryType::READ)
    throw std::invalid_argument(
        "Cannot read axis data; axis array is not opened in 'read' mode.");
  query_.set_subarray(subarray_.subarray());
}

Status UnorderedAxisQuery::cancel() {
  return query_.cancel();
}

Status UnorderedAxisQuery::finalize() {
  return query_.finalize();
}

Status UnorderedAxisQuery::init() {
  return query_.init();
}

Status UnorderedAxisQuery::set_data_buffer(
    const std::string& name,
    void* const buffer,
    uint64_t* const buffer_size,
    const bool check_null_buffers) {
  return query_.set_data_buffer(name, buffer, buffer_size, check_null_buffers);
}

Status UnorderedAxisQuery::set_offsets_buffer(
    const std::string& name,
    uint64_t* const buffer_offsets,
    uint64_t* const buffer_offsets_size,
    const bool check_null_buffers) {
  return query_.set_offsets_buffer(
      name, buffer_offsets, buffer_offsets_size, check_null_buffers);
}

Status UnorderedAxisQuery::set_validity_buffer(
    const std::string& name,
    uint8_t* const buffer_validity_bytemap,
    uint64_t* const buffer_validity_bytemap_size,
    const bool check_null_buffers) {
  return query_.set_validity_buffer(
      name,
      buffer_validity_bytemap,
      buffer_validity_bytemap_size,
      check_null_buffers);
}

/** Returns the query status. */
QueryStatus UnorderedAxisQuery::status() const {
  return query_.status();
}

/** Submits the query to the storage manager. */
Status UnorderedAxisQuery::submit() {
  return query_.submit();
}

}  // namespace tiledb::sm
