#include "tiledb/sm/label_query/label_query.h"
#include "tiledb/common/common.h"
#include "tiledb/sm/array/array.h"
#include "tiledb/sm/enums/query_type.h"
#include "tiledb/sm/label_query/axis_query.h"
#include "tiledb/sm/label_query/label_subarray.h"
#include "tiledb/sm/query/query.h"
#include "tiledb/sm/subarray/subarray.h"

using namespace tiledb::common;

namespace tiledb::sm {

LabelledQuery::LabelledQuery(
    const LabelledSubarray& subarray,
    StorageManager* storage_manager,
    Array* array,
    URI fragment_uri)
    : storage_manager_{storage_manager}
    , query_{storage_manager, array, fragment_uri}
    , dim_num_{query_.array_schema().dim_num()}
    , dimension_label_queries_{dim_num_, nullptr} {
  if (!array->is_open())
    throw std::invalid_argument("Cannot query array; array is not open.");
  throw_if_not_ok(array->get_query_type(&type_));
  // TODO Add subarray to constructor. Add construction of label queries.
  for (unsigned dim_idx{0}; dim_idx < dim_num_; ++dim_idx) {
    auto axis_subarray = subarray.label_subarray(dim_idx);
    if (axis_subarray.has_value()) {
      const auto order_type = axis_subarray->label_order_type();
      switch (order_type) {
        case LabelOrderType::UNORDERED:
          dimension_label_queries_[dim_idx] = make_shared<UnorderedAxisQuery>(
              HERE(), axis_subarray.value(), storage_manager);
          break;
        case LabelOrderType::REVERSE:
          throw std::invalid_argument(
              "Support for reverse ordered labels is not yet "
              "implemented.");
          break;
        case LabelOrderType::FORWARD:
          throw std::invalid_argument(
              "Support for ordered labels is not yet "
              "implemented.");
          break;
        default:
          throw std::invalid_argument("Invalid label order type.");
      }
    }
  }
}

Status LabelledQuery::cancel() {
  RETURN_NOT_OK(query_.cancel());
  for (auto& label_query : dimension_label_queries_) {
    if (label_query)
      RETURN_NOT_OK(label_query->cancel());
  }
  return Status::Ok();
}

Status LabelledQuery::finalize() {
  return query_.finalize();
}

Status LabelledQuery::finalize_labels() {
  for (auto& label_query : dimension_label_queries_) {
    if (label_query)
      RETURN_NOT_OK(label_query->finalize());
  }
  return Status::Ok();
}

// Status get_est_result_size(const char* name, uint64_t* size);

// Status get_est_result_size(
//     const char* name, uint64_t* size_off, uint64_t* size_val);

// Status get_est_result_size_nullable(
//     const char* name, uint64_t* size_val, uint64_t* size_validity);

// Status get_est_result_size_nullable(
//     const char* name,
//     uint64_t* size_off,
//     uint64_t* size_val,
//     uint64_t* size_validity);

// Status get_data_buffer(
//     const char* name, void** buffer, uint64_t** buffer_size) const;

// Status get_offsets_buffer(
//     const char* name, uint64_t** buffer_off, uint64_t**
//     buffer_off_size) const;

// Status get_validity_buffer(
//     const char* name,
//     uint8_t** buffer_validity_bytemap,
//     uint64_t** buffer_validity_bytemap_size) const;

// bool has_results() const;

Status LabelledQuery::init() {
  for (auto& label_query : dimension_label_queries_) {
    if (label_query && label_query->status() != QueryStatus::COMPLETED)
      return Status_LabelledQueryError(
          "Unable to initialize query until all label queries are "
          "completed.");
  }
  return query_.init();
}

//   Status set_data_buffer(
//       const std::string& name,
//       void* const buffer,
//       uint64_t* const buffer_size,
//       const bool check_null_buffers = true);

// Status set_offsets_buffer(
//     const std::string& name,
//     uint64_t* const buffer_offsets,
//     uint64_t* const buffer_offsets_size,
//     const bool check_null_buffers = true);

// Status set_offsets_buffer(
//     const std::string& name,
//     uint64_t* const buffer_offsets,
//     uint64_t* const buffer_offsets_size,
//     const bool check_null_buffers = true);

// Status set_validity_buffer(
//     const std::string& name,
//     uint8_t* const buffer_validity_bytemap,
//     uint64_t* const buffer_validity_bytemap_size,
//     const bool check_null_buffers = true);

// Status set_labelled_subarray(const LabelledSubarray& subarray);

// Status set_subarray(const Subarray& subarray);

// Status submit();

}  // namespace tiledb::sm
