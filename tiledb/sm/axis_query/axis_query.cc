#include "tiledb/sm/axis_query/axis_query.h"
#include "tiledb/sm/array/array.h"
#include "tiledb/sm/array_schema/attribute.h"
#include "tiledb/sm/array_schema/dimension.h"
#include "tiledb/sm/axis/axis.h"
#include "tiledb/sm/enums/datatype.h"
#include "tiledb/sm/enums/query_status.h"
#include "tiledb/sm/query/query.h"
#include "tiledb/sm/storage_manager/storage_manager.h"

namespace tiledb::sm {

/**********************/
/* OrderedAxisQuery */
/**********************/

OrderedAxisQuery::OrderedAxisQuery(
    shared_ptr<Axis> axis,
    StorageManager* storage_manager,
    const std::string& label_name)
    : axis_{axis}
    , storage_manager_{storage_manager}
    , label_name_{label_name}
    , internal_label_name_{axis->label_dimension()->name()}
    , internal_index_name_{axis->index_attribute()->name()}
    , range_query_{nullptr} {
}

Status OrderedAxisQuery::add_range(
    const void* start, const void* end, const void* stride) {
  if (stride != nullptr)
    return Status_AxisQueryError(
        "Cannot add range; Setting label range stride is currently "
        "unsupported.");
  if (range_query_ != nullptr)
    return Status_AxisQueryError(
        "Cannot add range; Setting more than one label range is currently "
        "unsupported.");
  range_query_ = tdb_unique_ptr<Query>(
      tdb_new(Query, storage_manager_, axis_->labelled_array()));
  return range_query_->add_range(0, start, end, stride);
}

Status OrderedAxisQuery::add_range_var(
    const void*, uint64_t, const void*, uint64_t) {
  return Status_AxisQueryError(
      "Adding variable length ranges in not yet supported for ordered "
      "labels.");
}

Status OrderedAxisQuery::cancel() {
  if (!range_query_)
    return Status::Ok();
  return range_query_->cancel();
}

Status OrderedAxisQuery::finalize() {
  if (!range_query_)
    return Status::Ok();
  return range_query_->finalize();
}

tuple<Status, void*, uint64_t> OrderedAxisQuery::get_index_point_ranges()
    const {
  void* start;
  uint64_t* size;
  auto status = range_query_->get_data_buffer(
      internal_index_name_.c_str(), &start, &size);
  Datatype index_type = range_query_->array_schema().type(internal_index_name_);
  uint64_t count = *size / datatype_size(index_type);
  return {status, start, count};
}

Status OrderedAxisQuery::init() {
  if (!range_query_)
    return Status::Ok();
  return range_query_->init();
}

Status OrderedAxisQuery::set_index_data_buffer(
    void* const buffer,
    uint64_t* const buffer_size,
    const bool check_null_buffers) {
  return range_query_->set_data_buffer(
      internal_index_name_, buffer, buffer_size, check_null_buffers);
}

Status OrderedAxisQuery::set_label_data_buffer(
    void* const buffer,
    uint64_t* const buffer_size,
    const bool check_null_buffers) {
  if (!range_query_)
    return Status::Ok();
  return range_query_->set_data_buffer(
      internal_label_name_, buffer, buffer_size, check_null_buffers);
}

/** Returns the query status. */
QueryStatus OrderedAxisQuery::status() const {
  if (!range_query_)
    return QueryStatus::COMPLETED;
  return range_query_->status();
}

/** Submits the query to the storage manager. */
Status OrderedAxisQuery::submit() {
  if (!range_query_)
    return Status::Ok();
  return range_query_->submit();
}

}  // namespace tiledb::sm
