#ifndef TILEDB_LABEL_SUBARRAY_H
#define TILEDB_LABEL_SUBARRAY_H

#include <unordered_map>
#include <vector>
#include "tiledb/common/common.h"
#include "tiledb/common/logger_public.h"
#include "tiledb/sm/enums/query_status.h"
#include "tiledb/sm/label_query/axis_subarray.h"
#include "tiledb/sm/subarray/subarray.h"

using namespace tiledb::common;

namespace tiledb::sm {

class StorageManager;

/** Return a Status_LabelledSubarray error class Status with a given message **/
inline Status Status_LabelledSubarrayError(const std::string& msg) {
  return {"[TileDB::LabelledSubarray] Error", msg};
}

class LabelledSubarray {
 public:
  /** Default constructor is not C.41 compliant. */
  LabelledSubarray() = delete;

  /**
   * Constructor.
   *
   * @param array The array the subarray is associated with.
   * @param parent_stats The parent stats to inherit from.
   * @param logger The parent logger to clone and use for logging
   * @param coalesce_ranges When enabled, ranges will attempt to coalesce
   *     with existing ranges as they are added.
   */
  LabelledSubarray(
      const Array* array,
      stats::Stats* parent_stats,
      shared_ptr<Logger> logger,
      bool coalesce_ranges = true,
      StorageManager* storage_manager = nullptr);

  /**
   * Constructor.
   *
   * @param array The array the subarray is associated with.
   * @param layout The layout of the values of the subarray (of the results
   *     if the subarray is used for reads, or of the values provided
   *     by the user for writes).
   * @param parent_stats The parent stats to inherit from.
   * @param logger The parent logger to clone and use for logging
   * @param coalesce_ranges When enabled, ranges will attempt to coalesce
   *     with existing ranges as they are added.
   */
  LabelledSubarray(
      const Array* array,
      Layout layout,
      stats::Stats* parent_stats,
      shared_ptr<Logger> logger,
      bool coalesce_ranges = true,
      StorageManager* storage_manager = nullptr);

  /**
   * Adds a range to the subarray on the input dimension by index,
   * in the form of (start, end, stride).
   * The range components must be of the same type as the domain type of the
   * underlying array.
   *
   * This is called directly by the C-API.
   *
   * TODO: add params to docs.
   */
  Status add_range(
      unsigned dim_idx, const void* start, const void* end, const void* stride);

  /**
   * Set point ranges from an array
   *
   * @param dim_idx Dimension index
   * @param start Pointer to start of the array
   * @param count Number of elements to add
   * @return Status
   */
  Status add_point_ranges(unsigned dim_idx, const void* start, uint64_t count);

  /**
   * Adds a variable-sized range to the (read/write) query on the input
   * dimension by index, in the form of (start, end).
   */
  Status add_range_var(
      unsigned dim_idx,
      const void* start,
      uint64_t start_size,
      const void* end,
      uint64_t end_size);

  /** Retrieves the number of ranges on the given dimension index. */
  Status get_range_num(uint32_t dim_idx, uint64_t* range_num) const;

  /**
   * Retrieves a range from a dimension index in the form (start, end, stride).
   *
   * @param dim_idx The dimension to retrieve the range from.
   * @param range_idx The id of the range to retrieve.
   * @param start The range start to retrieve.
   * @param end The range end to retrieve.
   * @param stride The range stride to retrieve.
   * @return Status
   */
  Status get_range(
      unsigned dim_idx,
      uint64_t range_idx,
      const void** start,
      const void** end,
      const void** stride) const;

  /**
   * Retrieves a range from a variable-length dimension index in the form
   * (start, end).
   *
   * @param dim_idx The dimension to retrieve the range from.
   * @param range_idx The id of the range to retrieve.
   * @param start The range start to retrieve.
   * @param end The range end to retrieve.
   * @return Status
   */
  Status get_range_var(
      unsigned dim_idx, uint64_t range_idx, void* start, void* end) const;

  /**
   * Retrieves a range's start and end size for a given variable-length
   * dimensions at a given range index.
   *
   *
   * @param dim_idx The dimension to retrieve the range from.
   * @param range_idx The id of the range to retrieve.
   * @param start_size range start size in bytes
   * @param end_size range end size in bytes
   * @return Status
   */
  Status get_range_var_size(
      uint32_t dim_idx,
      uint64_t range_idx,
      uint64_t* start_size,
      uint64_t* end_size) const;

  /**
   * TODO Add documentation
   */
  inline bool is_labelled(const unsigned dim_idx) const {
    return label_subarrays_.at(dim_idx).has_value();
  }

  /**
   * TODO Change to be [] operator?
   */
  inline optional<AxisSubarray> label_subarray(const unsigned dim_idx) const {
    return label_subarrays_[dim_idx];
  }

  /**
   * TODO Add documentation
   */
  Status set_external_label(
      const unsigned dim_idx,
      const std::string& label_name,
      const LabelOrderType order_type,
      const std::string& internal_label_name,
      const std::string& internal_index_name,
      const Array* array);

  /**
   * TODO Add documentation
   */
  Status set_label(const unsigned dim_idx, const unsigned label_idx);

  // /**
  //  * TODO Add documentation
  //  */
  // Status set_label_by_name(label_name);

  /**
   * Sets config for query-level parameters only.
   *
   * Part of the C-API.
   **/
  Status set_config(const Config& config);

  /**
   * Get the config of the writer
   * @return Config
   */
  const Config* config() const;

  /**
   * Sets coalesce_ranges flag, intended for use by CAPI, to alloc matching
   * default coalesce-ranges=true semantics of internal class constructor, but
   * giving capi clients ability to turn off if desired.
   */
  Status set_coalesce_ranges(bool coalesce_ranges = true);

 private:
  stats::Stats* stats_;
  inline static std::atomic<uint64_t> logger_id_ = 0;
  shared_ptr<Logger> logger_;
  bool coalesce_ranges_;
  Subarray subarray_;
  std::vector<optional<AxisSubarray>> label_subarrays_;
};

}  // namespace tiledb::sm
#endif
