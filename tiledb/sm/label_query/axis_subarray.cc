#include "tiledb/common/logger.h"
#include "tiledb/sm/array/array.h"
#include "tiledb/sm/config/config.h"
#include "tiledb/sm/enums/layout.h"
#include "tiledb/sm/label_query/label_subarray.h"
#include "tiledb/sm/stats/stats.h"
#include "tiledb/sm/storage_manager/storage_manager.h"

using namespace tiledb::common;

namespace tiledb::sm {

AxisSubarray::AxisSubarray(
    const std::string& label_name,
    const LabelOrderType order_type,
    const std::string& internal_label_name,
    const std::string& internal_index_name,
    const Array* array,
    const Layout layout,
    stats::Stats* stats,
    shared_ptr<Logger> logger,
    const bool coalesce_ranges)
    : subarray_(array, layout, stats, logger, coalesce_ranges, nullptr)
    , label_name_(label_name)
    , order_type_(order_type)
    , internal_label_name_(internal_label_name)
    , internal_index_name_(internal_index_name) {
  if (order_type == LabelOrderType::UNORDERED && layout != Layout::UNORDERED)
    throw std::invalid_argument(
        "Cannot create axis subarray; Support for unordered layouts on "
        "unordered subarrays are not yet implemented.");
  const ArraySchema& array_schema = array->array_schema_latest();
  if (array_schema.dim_num() != 1)
    throw std::invalid_argument(
        "Cannot create axis subarray; Array must be one dimensional.");
  if (!array_schema.is_dim(internal_label_name))
    throw std::invalid_argument(
        "Cannot create axis query; Expected dimension " + internal_label_name +
        "'.");
  if (!array_schema.is_attr(internal_index_name))
    throw std::invalid_argument(
        "Cannot create axis query; Array is missing index attribute "
        "'" +
        internal_index_name + "'.");
}

}  // namespace tiledb::sm
