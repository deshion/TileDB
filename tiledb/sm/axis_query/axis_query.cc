#include "tiledb/sm/axis_query/axis_query.h"
#include "tiledb/sm/array/array.h"
#include "tiledb/sm/enums/datatype.h"
#include "tiledb/sm/query/query.h"
#include "tiledb/sm/storage_manager/storage_manager.h"

namespace tiledb::sm {

/**********************/
/* UnorderedAxisQuery */
/**********************/

UnorderedAxisQuery::UnorderedAxisQuery(
    Array* array,
    StorageManager* storage_manager,
    const std::string& label_name,
    const std::string& internal_label_name,
    const std::string& internal_index_name)
    : query_{storage_manager, array}
    , label_name_{label_name}
    , internal_label_name_{internal_label_name}
    , internal_index_name_{internal_index_name} {
}

Status UnorderedAxisQuery::cancel() {
  return query_.cancel();
}

Status UnorderedAxisQuery::finalize() {
  return query_.finalize();
}

tuple<Status, void*, uint64_t> UnorderedAxisQuery::get_index_point_ranges()
    const {
  void* start;
  uint64_t* size;
  auto status =
      query_.get_data_buffer(internal_index_name_.c_str(), &start, &size);
  Datatype index_type = query_.array_schema().type(internal_index_name_);
  uint64_t count = *size / datatype_size(index_type);
  return {status, start, count};
}

Status UnorderedAxisQuery::init() {
  return query_.init();
}

Status UnorderedAxisQuery::set_index_data_buffer(
    void* const buffer,
    uint64_t* const buffer_size,
    const bool check_null_buffers) {
  return query_.set_data_buffer(
      internal_index_name_, buffer, buffer_size, check_null_buffers);
}

Status UnorderedAxisQuery::set_label_data_buffer(
    void* const buffer,
    uint64_t* const buffer_size,
    const bool check_null_buffers) {
  return query_.set_data_buffer(
      internal_label_name_, buffer, buffer_size, check_null_buffers);
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
