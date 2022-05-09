#include "tiledb/sm/label_query/axis_query.h"
#include "tiledb/sm/array/array.h"
#include "tiledb/sm/query/query.h"
#include "tiledb/sm/storage_manager/storage_manager.h"

namespace tiledb::sm {

/*************/
/* AxisQuery */
/*************/

AxisQuery::AxisQuery(
    const QueryType type, StorageManager* storage_manager, Array* array)
    : query_{storage_manager, array} {
  if (!array->is_open())
    throw std::invalid_argument("Cannot query axis; array is not open.");
  QueryType actual_type;
  throw_if_not_ok(query_.array()->get_query_type(&actual_type));
  if (actual_type != type)
    throw std::invalid_argument(
        "Cannot query axis; array is opened with an unexpected type.");
}

/********************/
/* ForwardAxisQuery */
/********************/
ForwardAxisQuery::ForwardAxisQuery(
    const std::string& label_name,
    const std::string& index_name,
    StorageManager* storage_manager,
    Array* array)
    : AxisQuery(QueryType::READ, storage_manager, array)
    , index_name_{index_name} {
  const ArraySchema& array_schema = array->array_schema_latest();
  if (array_schema.dim_num() != 1)
    throw std::invalid_argument(
        "Cannot create ordered axis query; Array must be an sparse one "
        "dimensional.");
  if (!array_schema.is_dim(label_name))
    throw std::invalid_argument(
        "Cannot create ordered axis query; Expected dimension " + label_name +
        "'.");
  if (!array_schema.is_attr(index_name))
    throw std::invalid_argument(
        "Cannot create ordered axis query; Array is missing index attribute '" +
        index_name + "'.");
}

/********************/
/* ReverseAxisQuery */
/********************/

ReverseAxisQuery::ReverseAxisQuery(
    const std::string& label_name,
    const std::string& index_name,
    StorageManager* storage_manager,
    Array* array)
    : AxisQuery(QueryType::READ, storage_manager, array)
    , index_name_{index_name} {
  const ArraySchema& array_schema = array->array_schema_latest();
  if (array_schema.dim_num() != 1)
    throw std::invalid_argument(
        "Cannot create ordered axis query; Array must be an sparse one "
        "dimensional.");
  if (!array_schema.is_dim(label_name))
    throw std::invalid_argument(
        "Cannot create ordered axis query; Expected dimension " + label_name +
        "'.");
  if (!array_schema.is_attr(index_name))
    throw std::invalid_argument(
        "Cannot create ordered axis query; Array is missing index attribute '" +
        index_name + "'.");
}

/**********************/
/* UnorderedAxisQuery */
/**********************/

UnorderedAxisQuery::UnorderedAxisQuery(
    const std::string& label_name,
    const std::string& index_name,
    StorageManager* storage_manager,
    Array* array)
    : AxisQuery(QueryType::READ, storage_manager, array)
    , index_name_{index_name} {
  const ArraySchema& array_schema = array->array_schema_latest();
  if (array_schema.dim_num() != 1)
    throw std::invalid_argument(
        "Cannot create unordered axis query; Array must be one "
        "dimensional.");
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

/*****************/
/* DataAxisQuery */
/*****************/
DataAxisQuery::DataAxisQuery(
    const std::string& label_name,
    const std::string& index_name,
    StorageManager* storage_manager,
    Array* array)
    : AxisQuery(QueryType::READ, storage_manager, array)
    , label_name_{label_name} {
  const ArraySchema& array_schema = array->array_schema_latest();
  if (array_schema.dim_num() != 1)
    throw std::invalid_argument(
        "Cannot create data axis query; Array must be one "
        "dimensional.");
  if (!array_schema.is_dim(index_name))
    throw std::invalid_argument(
        "Cannot create data axis query; Expected dimension " + index_name +
        "'.");
  if (!array_schema.is_attr(label_name))
    throw std::invalid_argument(
        "Cannot create data axis query; Array is missing index attribute '" +
        label_name + "'.");
}

}  // namespace tiledb::sm
