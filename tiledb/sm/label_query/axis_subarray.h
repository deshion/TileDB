#ifndef TILEDB_AXIS_SUBARRAY_H
#define TILEDB_AXIS_SUBARRAY_H

#include "tiledb/common/common.h"
#include "tiledb/common/logger_public.h"
#include "tiledb/sm/subarray/subarray.h"
using namespace tiledb::common;

namespace tiledb::sm {

class StorageManager;

/** Return a Status_ArrayLabelsError error class Status with a given message **/
inline Status Status_AxisSubarrayError(const std::string& msg) {
  return {"[TileDB::AxisSubarray] Error", msg};
}

enum class LabelOrderType : uint8_t { UNORDERED, FORWARD, REVERSE };

class AxisSubarray {
 public:
  AxisSubarray(
      const std::string& label_name,
      const LabelOrderType order_type,
      const std::string& internal_label_name,
      const std::string& internal_index_name,
      const Array* array,
      const Layout layout,
      stats::Stats* stats,
      shared_ptr<Logger> logger,
      const bool coalesce_ranges);

  inline tuple<std::string, std::string, std::string> access_signature() {
    return {label_name_, internal_label_name_, internal_index_name_};
  }

  inline Status add_range(
      unsigned dim_idx,
      const void* start,
      const void* end,
      const void* stride) {
    return subarray_.add_range(dim_idx, start, end, stride);
  }

  inline Status add_point_ranges(
      unsigned dim_idx, const void* start, uint64_t count) {
    return subarray_.add_point_ranges(dim_idx, start, count);
  }

  inline Status add_range_var(
      unsigned dim_idx,
      const void* start,
      uint64_t start_size,
      const void* end,
      uint64_t end_size) {
    return subarray_.add_range_var(dim_idx, start, start_size, end, end_size);
  }

  inline const Array* array() const {
    return subarray_.array();
  }

  inline Status get_range_num(uint32_t dim_idx, uint64_t* range_num) const {
    return subarray_.get_range_num(dim_idx, range_num);
  }

  Status get_range(
      unsigned dim_idx,
      uint64_t range_idx,
      const void** start,
      const void** end,
      const void** stride) const {
    return subarray_.get_range(dim_idx, range_idx, start, end, stride);
  }

  Status get_range_var(
      unsigned dim_idx, uint64_t range_idx, void* start, void* end) const {
    return subarray_.get_range_var(dim_idx, range_idx, start, end);
  }

  Status get_range_var_size(
      uint32_t dim_idx,
      uint64_t range_idx,
      uint64_t* start_size,
      uint64_t* end_size) const {
    return subarray_.get_range_var_size(
        dim_idx, range_idx, start_size, end_size);
  }

  inline LabelOrderType label_order_type() const {
    return order_type_;
  }

  inline const Subarray& subarray() const {
    return subarray_;
  }

  inline Status set_coalesce_ranges(bool coalesce_ranges) {
    return subarray_.set_coalesce_ranges(coalesce_ranges);
  }

  inline Status set_config(const Config& config) {
    return subarray_.set_config(config);
  }

 private:
  Subarray subarray_;
  std::string label_name_;
  LabelOrderType order_type_;
  std::string internal_label_name_;
  std::string internal_index_name_;
};

}  // namespace tiledb::sm

#endif
