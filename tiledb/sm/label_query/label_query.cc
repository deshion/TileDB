#include "tiledb/sm/label_query/label_query.h"
#include "tiledb/common/common.h"
#include "tiledb/sm/array/array.h"
#include "tiledb/sm/enums/query_type.h"
#include "tiledb/sm/label_query/axis_query.h"
#include "tiledb/sm/query/query.h"
#include "tiledb/sm/subarray/subarray.h"

using namespace tiledb::common;

namespace tiledb::sm {

LabelledQuery::LabelledQuery(
    StorageManager* storage_manager, Array* array, URI fragment_uri)
    : storage_manager_{storage_manager}
    , query_{storage_manager, array, fragment_uri}
    , dim_num_{query_.array_schema().dim_num()}
    , dimension_label_queries_{dim_num_, nullptr}
    , extra_label_queries_{dim_num_}
    , extra_dim_queries_{dim_num_} {
  if (!array->is_open())
    throw std::invalid_argument("Cannot query array; array is not open.");
  throw_if_not_ok(array->get_query_type(&type_));
}

void LabelledQuery::add_external_dimension_label(
    const unsigned dim_idx,
    const LabelOrderType order_type,
    const std::string& internal_label_name,
    const std::string& internal_index_name,
    Array* array) {
  if (dim_idx > dim_num_)
    throw std::invalid_argument(
        "Cannot add label to query; Invalid dimension index.");
  if (dimension_label_queries_[dim_idx] != nullptr)
    throw std::invalid_argument(
        "Cannot add label to query; Dimension label already set for "
        "dimension " +
        std::to_string(dim_idx) + ".");
  switch (order_type) {
    case (LabelOrderType::UNORDERED):
      dimension_label_queries_[dim_idx] = make_shared<UnorderedAxisQuery>(
          HERE(),
          internal_label_name,
          internal_index_name,  // TODO: Add index datatype info for checks
          storage_manager_,
          array);
      break;
      //    case (LabelOrderType::FORWARD):
      //      dimension_label_queries_[dim_idx] = make_shared<ForwardAxisQuery>(
      //          HERE(),
      //          internal_label_name,
      //          internal_index_name,
      //          storage_manager_,
      //          array);
      //      break;
      //    case (LabelOrderType::REVERSE):
      //      dimension_label_queries_[dim_idx] = make_shared<ReverseAxisQuery>(
      //          HERE(),
      //          internal_label_name,
      //          internal_index_name,
      //          storage_manager_,
      //          array);
      //      break;
    default:
      std::invalid_argument("Cannot add label; invalid label order type.");
  }
}

// Status add_range(
//     unsigned dim_idx, const void* start, const void* end, const void* stride)
//     {
// }
//
// Status add_range_var(
//     unsigned dim_idx,
//     const void* start,
//     uint64_t start_size,
//     const void* end,
//     uint64_t end_size);
//
// Status cancel();
//
// Status finalize();
//
// Status get_est_result_size(const char* name, uint64_t* size);
//
// Status get_est_result_size(
//     const char* name, uint64_t* size_off, uint64_t* size_val);
//
// Status get_est_result_size_nullable(
//     const char* name, uint64_t* size_val, uint64_t* size_validity);
//
// Status get_est_result_size_nullable(
//     const char* name,
//     uint64_t* size_off,
//     uint64_t* size_val,
//     uint64_t* size_validity);
//
// Status get_data_buffer(
//     const char* name, void** buffer, uint64_t** buffer_size) const;
//
// Status get_offsets_buffer(
//     const char* name, uint64_t** buffer_off, uint64_t** buffer_off_size)
//     const;
//
// Status get_validity_buffer(
//     const char* name,
//     uint8_t** buffer_validity_bytemap,
//     uint64_t** buffer_validity_bytemap_size) const;
//
// bool has_results() const;
//
// Status init();
//
// Status set_data_buffer(
//     const std::string& name,
//     void* const buffer,
//     uint64_t* const buffer_size,
//     const bool check_null_buffers = true);
//
// Status set_offsets_buffer(
//     const std::string& name,
//     uint64_t* const buffer_offsets,
//     uint64_t* const buffer_offsets_size,
//     const bool check_null_buffers = true);
//
// Status set_offsets_buffer(
//     const std::string& name,
//     uint64_t* const buffer_offsets,
//     uint64_t* const buffer_offsets_size,
//     const bool check_null_buffers = true);
//
// Status set_validity_buffer(
//     const std::string& name,
//     uint8_t* const buffer_validity_bytemap,
//     uint64_t* const buffer_validity_bytemap_size,
//     const bool check_null_buffers = true);
//
// Status set_layout(Layout layout);
//
// Status set_subarray(const void* subarray);
//
// Status submit();
//
// Status submit_async(std::function<void(void*)> callback, void*
// callback_data);

}  // namespace tiledb::sm
