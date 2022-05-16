#include "tiledb/sm/label_query/label_subarray.h"
#include "tiledb/common/logger.h"
#include "tiledb/sm/array/array.h"
#include "tiledb/sm/config/config.h"
#include "tiledb/sm/enums/layout.h"
#include "tiledb/sm/label_query/axis_subarray.h"
#include "tiledb/sm/stats/stats.h"
#include "tiledb/sm/storage_manager/storage_manager.h"

using namespace tiledb::common;

namespace tiledb::sm {

LabelledSubarray::LabelledSubarray(
    const Array* array,
    stats::Stats* const parent_stats,
    shared_ptr<Logger> logger,
    const bool coalesce_ranges,
    StorageManager* storage_manager)
    : LabelledSubarray(
          array,
          Layout::UNORDERED,
          parent_stats,
          logger,
          coalesce_ranges,
          storage_manager) {
}

LabelledSubarray::LabelledSubarray(
    const Array* const array,
    const Layout layout,
    stats::Stats* const parent_stats,
    shared_ptr<Logger> logger,
    const bool coalesce_ranges,
    StorageManager* storage_manager)
    : stats_(
          parent_stats ? parent_stats->create_child("Subarray") :
                         storage_manager ?
                         storage_manager->stats()->create_child("subSubarray") :
                         nullptr)
    , logger_(logger->clone("LabelledSubarray", ++logger_id_))
    , coalesce_ranges_(coalesce_ranges)
    , subarray_(array, layout, stats_, logger_, coalesce_ranges)
    , label_subarrays_(array->array_schema_latest().dim_num(), nullopt) {
  if (!parent_stats && !storage_manager)
    throw std::runtime_error(
        "Subarray(): missing parent_stats requires live storage_manager!");
}

Status LabelledSubarray::add_range(
    unsigned dim_idx, const void* start, const void* end, const void* stride) {
  if (is_labelled(dim_idx))
    return label_subarrays_[dim_idx]->add_range(0, start, end, stride);
  return subarray_.add_range(dim_idx, start, end, stride);
}

Status LabelledSubarray::add_point_ranges(
    unsigned dim_idx, const void* start, uint64_t count) {
  if (is_labelled(dim_idx))
    return label_subarrays_[dim_idx]->add_point_ranges(0, start, count);
  return subarray_.add_point_ranges(dim_idx, start, count);
}

Status LabelledSubarray::add_range_var(
    unsigned dim_idx,
    const void* start,
    uint64_t start_size,
    const void* end,
    uint64_t end_size) {
  if (is_labelled(dim_idx))
    return label_subarrays_[dim_idx]->add_range_var(
        0, start, start_size, end, end_size);
  return subarray_.add_range_var(dim_idx, start, start_size, end, end_size);
}

Status LabelledSubarray::get_range_num(
    uint32_t dim_idx, uint64_t* range_num) const {
  if (is_labelled(dim_idx))
    return label_subarrays_[dim_idx]->get_range_num(0, range_num);
  return subarray_.get_range_num(dim_idx, range_num);
}

Status LabelledSubarray::get_range(
    unsigned dim_idx,
    uint64_t range_idx,
    const void** start,
    const void** end,
    const void** stride) const {
  if (is_labelled(dim_idx))
    return label_subarrays_[dim_idx]->get_range(
        0, range_idx, start, end, stride);
  return subarray_.get_range(dim_idx, range_idx, start, end, stride);
}

Status LabelledSubarray::get_range_var(
    unsigned dim_idx, uint64_t range_idx, void* start, void* end) const {
  if (is_labelled(dim_idx))
    return label_subarrays_[dim_idx]->get_range_var(0, range_idx, start, end);
  return subarray_.get_range_var(dim_idx, range_idx, start, end);
}

Status LabelledSubarray::get_range_var_size(
    uint32_t dim_idx,
    uint64_t range_idx,
    uint64_t* start_size,
    uint64_t* end_size) const {
  if (is_labelled(dim_idx))
    return label_subarrays_[dim_idx]->get_range_var_size(
        0, range_idx, start_size, end_size);
  return subarray_.get_range_var_size(dim_idx, range_idx, start_size, end_size);
}

Status LabelledSubarray::set_external_label(
    const unsigned dim_idx,
    const std::string& label_name,
    const LabelOrderType order_type,
    const std::string& internal_label_name,
    const std::string& internal_index_name,
    const Array* array) {
  if (is_labelled(dim_idx))
    return Status_LabelledSubarrayError(
        "Cannot set label on dimension " + std::to_string(dim_idx) +
        "; A dimension label is already set on this dimension.");
  if (subarray_.is_set(dim_idx))
    return Status_LabelledSubarrayError(
        "Cannot set label on dimension " + std::to_string(dim_idx) +
        "; The dimension already has ranges added to it.");
  label_subarrays_[dim_idx] = AxisSubarray(
      label_name,
      order_type,
      internal_label_name,
      internal_index_name,
      array,
      subarray_.layout(),
      stats_,
      logger_,
      coalesce_ranges_);
  return Status::Ok();
}

Status LabelledSubarray::set_label(
    const unsigned dim_idx, const unsigned label_idx) {
  if (is_labelled(dim_idx))
    return Status_LabelledSubarrayError(
        "Cannot set label on dimension " + std::to_string(dim_idx) +
        "; Dimension label is already set.");
  if (subarray_.is_set(dim_idx))
    return Status_LabelledSubarrayError(
        "Cannot set label on dimension " + std::to_string(dim_idx) +
        "; The dimension already has ranges set.");
  return Status_LabelledSubarrayError(
      "Cannot set label " + std::to_string(label_idx) + " on dimension " +
      std::to_string(dim_idx) +
      ". Support for internal labels is not yet implemented.");
}

Status LabelledSubarray::set_config(const Config& config) {
  RETURN_NOT_OK(subarray_.set_config(config));
  for (auto& label_subarray : label_subarrays_) {
    if (label_subarray.has_value()) {
      RETURN_NOT_OK(label_subarray->set_config(config));
    }
  }
  return Status::Ok();
}

const Config* LabelledSubarray::config() const {
  return subarray_.config();
}

Status LabelledSubarray::set_coalesce_ranges(bool coalesce_ranges) {
  // TODO: Fix so if failed, action is undone.
  RETURN_NOT_OK(subarray_.set_coalesce_ranges(coalesce_ranges));
  for (auto& label_subarray : label_subarrays_) {
    if (label_subarray.has_value()) {
      RETURN_NOT_OK(label_subarray->set_coalesce_ranges(coalesce_ranges));
    }
  }
  return Status::Ok();
}

void LabelledSubarray::set_layout(Layout layout) {
  subarray_.set_layout(layout);
  for (auto& label_subarray : label_subarrays_) {
    if (label_subarray.has_value())
      label_subarray->set_layout(layout);
  }
}

}  // namespace tiledb::sm
