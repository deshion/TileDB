#include "tiledb/sm/label_query/axis_query.h"
#include "tiledb/sm/array/array.h"
#include "tiledb/sm/query/query.h"
#include "tiledb/sm/storage_manager/storage_manager.h"

namespace tiledb::sm {

/**********************/
/* UnorderedAxisQuery */
/**********************/

UnorderedAxisQuery::UnorderedAxisQuery(
    shared_ptr<UnorderedAxisSubarray> subarray,
    StorageManager* storage_manager,
    Array* array)
    : query_{storage_manager, subarray->array}
    , subarray_{subarray} {
  if (!array->is_open())
    throw std::invalid_argument("Cannot query axis; array is not open.");
  QueryType actual_type;
  throw_if_not_ok(query_.array()->get_query_type(&actual_type));
  if (actual_type != QueryType::READ)
    throw std::invalid_argument(
        "Cannot query axis; array is not opened in 'read' mode.");
}

}  // namespace tiledb::sm
