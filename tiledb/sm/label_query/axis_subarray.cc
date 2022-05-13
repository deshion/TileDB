#include "tiledb/common/logger.h"
#include "tiledb/sm/array/array.h"
#include "tiledb/sm/config/config.h"
#include "tiledb/sm/enums/layout.h"
#include "tiledb/sm/label_query/label_subarray.h"
#include "tiledb/sm/stats/stats.h"
#include "tiledb/sm/storage_manager/storage_manager.h"

using namespace tiledb::common;

namespace tiledb::sm {

// TODO: Create AxisSubarray base class

UnorderedAxisSubarray::UnorderedAxisSubarray(
    const std::string& label_name,
    const std::string& internal_label_name,
    const std::string& internal_index_name,
    const Array* array,
    const Layout layout,
    stats::Stats* stats,
    shared_ptr<Logger> logger,
    const bool coalesce_ranges)
    : subarray(array, layout, stats, logger, coalesce_ranges, nullptr)
    , label_name_(label_name)
    , internal_label_name_(internal_label_name)
    , internal_index_name_(internal_index_name) {
  if (layout != Layout::UNORDERED)
    throw std::invalid_argument(
        "Cannot create axis subarray; Support for unordered layouts on "
        "unordered subarrays are not yet implemented.");
  const ArraySchema& array_schema = array->array_schema_latest();
  if (array_schema.dim_num() != 1)
    throw std::invalid_argument(
        "Cannot create axis subarray; array must be one dimensional.");
  if (!array_schema.is_dim(label_name))
    throw std::invalid_argument(
        "Cannot create unordered axis query; Expected dimension " + label_name +
        "'.");
  if (!array_schema.is_attr(index_name))
    throw std::invalid_argument(
        "Cannot create unordered axis query; Array is missing index attribute "
        "'" +
        index_name + "'.");
}

}  // namespace tiledb::sm
