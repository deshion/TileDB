#include "tiledb/sm/label_query/label_query.h"
#include "tiledb/common/common.h"
#include "tiledb/sm/array/array.h"
#include "tiledb/sm/enums/query_status.h"
#include "tiledb/sm/label_query/axis_query.h"
#include "tiledb/sm/label_query/label_subarray.h"
#include "tiledb/sm/misc/constants.h"
#include "tiledb/sm/query/query.h"
#include "tiledb/sm/subarray/subarray.h"

#include <algorithm>

using namespace tiledb::common;

namespace tiledb::sm {

LabelledQuery::LabelledQuery(
    const LabelledSubarray& subarray,
    StorageManager* storage_manager,
    Array* array,
    URI fragment_uri)
    : storage_manager_{storage_manager}
    , query_{storage_manager, array, fragment_uri}
    , subarray_{subarray}
    , dim_num_{query_.array_schema().dim_num()}
    , label_queries_(dim_num_, nullptr)
    , labels_applied_(dim_num_, true) {
  if (!array->is_open())
    throw std::invalid_argument("Cannot query array; array is not open.");
  throw_if_not_ok(array->get_query_type(&type_));
  for (unsigned dim_idx{0}; dim_idx < dim_num_; ++dim_idx) {
    auto axis_subarray = subarray.label_subarray(dim_idx);
    if (axis_subarray.has_value()) {
      const auto order_type = axis_subarray->label_order_type();
      switch (order_type) {
        case LabelOrderType::UNORDERED:
          label_queries_[dim_idx] = make_shared<UnorderedAxisQuery>(
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
      labels_applied_[dim_idx] = false;
      label_map_[axis_subarray->label_name()] = label_queries_[dim_idx].get();
    }
  }
}

Status LabelledQuery::apply_label(const unsigned dim_idx) {
  if (labels_applied_[dim_idx])
    return Status::Ok();
  const auto& label_query = label_queries_[dim_idx];
  if (!label_query) {
    labels_applied_[dim_idx] = true;
    return Status::Ok();
  }
  if (label_query->status() != QueryStatus::COMPLETED)
    // Figure out how to log status immediately
    return Status_LabelledQueryError(
        "Cannot apply label on dimension " + std::to_string(dim_idx) +
        ". Label query is not completed.");
  // TODO Add method to subarray that clears the ranges. Call here.
  auto&& [status, start, count] = label_query->get_index_point_ranges();
  if (!status.ok())  // Figure out how to log status immediately
    return status;
  RETURN_NOT_OK(subarray_.add_point_ranges(dim_idx, start, count));
  labels_applied_[dim_idx] = true;
  return Status::Ok();
}

Status LabelledQuery::apply_labels() {
  bool success{true};
  for (unsigned dim_idx{0}; dim_idx < dim_num_; ++dim_idx) {
    auto status = apply_label(dim_idx);
    if (!status.ok())
      success = false;
  }
  return success ? Status::Ok() :
                   Status_LabelledQueryError(
                       "Unable to apply labels on all dimensions.");
}

Status LabelledQuery::cancel() {
  RETURN_NOT_OK(query_.cancel());
  for (auto& label_query : label_queries_) {
    if (label_query)
      RETURN_NOT_OK(label_query->cancel());
  }
  return Status::Ok();
}

Status LabelledQuery::finalize() {
  return query_.finalize();
}

Status LabelledQuery::finalize_labels() {
  for (auto& label_query : label_queries_) {
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

bool LabelledQuery::labels_applied() const {
  for (const auto& applied : labels_applied_) {
    if (!applied)
      return false;
  }
  return true;
}

Status LabelledQuery::set_data_buffer(
    const std::string& name,
    void* const buffer,
    uint64_t* const buffer_size,
    const bool check_null_buffers) {
  if (name == constants::coords)
    return Status_LabelledQueryError(
        "Cannot set zipped coordinates on a labelled query.");
  // If the buffer is for a labelled dimension, set the buffer to the label
  // query. Otherwise, set it to the main query.
  if (query_.array_schema().is_dim(name)) {
    unsigned dim_idx;
    RETURN_NOT_OK(
        query_.array_schema().domain().get_dimension_index(name, &dim_idx));
    if (label_queries_[dim_idx])
      return label_queries_[dim_idx]->set_index_data_buffer(
          buffer, buffer_size, check_null_buffers);
  }
  return query_.set_data_buffer(name, buffer, buffer_size, check_null_buffers);
}

Status LabelledQuery::set_label_data_buffer(
    const std::string& name,
    void* const buffer,
    uint64_t* const buffer_size,
    const bool check_null_buffers) {
  auto label_query = label_map_.find(name);
  if (label_query == label_map_.end())
    return Status_LabelledQueryError("No label with name '" + name + "'.");
  return label_query->second->set_label_data_buffer(
      buffer, buffer_size, check_null_buffers);
}

// Status set_offsets_buffer(
//     const std::string& name,
//     uint64_t* const buffer_offsets,
//     uint64_t* const buffer_offsets_size,
//     const bool check_null_buffers);

// Status set_offsets_buffer(
//     const std::string& name,
//     uint64_t* const buffer_offsets,
//     uint64_t* const buffer_offsets_size,
//     const bool check_null_buffers);

// Status set_validity_buffer(
//     const std::string& name,
//     uint8_t* const buffer_validity_bytemap,
//     uint64_t* const buffer_validity_bytemap_size,
//     const bool check_null_buffers);

// Status set_labelled_subarray(const LabelledSubarray& subarray);

// Status set_subarray(const Subarray& subarray);

Status LabelledQuery::submit_labels() {
  for (auto& label_query : label_queries_) {
    if (label_query)
      RETURN_NOT_OK(label_query->submit());
  }
  return Status::Ok();
}

Status LabelledQuery::submit() {
  if (!labels_applied())
    return Status_LabelledQueryError(
        "Unable to submit query until all label queries are "
        "completed.");
  RETURN_NOT_OK(query_.set_subarray(subarray_.subarray()));
  return query_.submit();
}

}  // namespace tiledb::sm
