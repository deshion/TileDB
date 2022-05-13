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

  inline const Array* array() const {
    return subarray.array();
  };

  Subarray subarray;

 private:
  std::string label_name_;
  std::string internal_label_name_;
  std::string internal_index_name_;
};

}  // namespace tiledb::sm

#endif
