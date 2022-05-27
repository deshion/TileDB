#include "tiledb/sm/axis_query/axis_query.h"
#include "tiledb/common/logger.h"
#include "tiledb/common/logger_public.h"
#include "tiledb/sm/array/array.h"
#include "tiledb/sm/array_schema/attribute.h"
#include "tiledb/sm/array_schema/dimension.h"
#include "tiledb/sm/axis/axis.h"
#include "tiledb/sm/enums/datatype.h"
#include "tiledb/sm/query/query.h"
#include "tiledb/sm/storage_manager/storage_manager.h"

namespace tiledb::sm {

OrderedAxisQuery::OrderedAxisQuery(
    shared_ptr<Axis> axis, StorageManager* storage_manager)
    : axis_{axis}
    , storage_manager_{storage_manager}
    , stats_{storage_manager_->stats()->create_child("AxisQuery")}
    , logger_(storage_manager->logger()->clone("AxisQuery", ++logger_id_))
    , range_query_{nullptr}
    , labelled_array_query_{nullptr}
    , indexed_array_query_{nullptr} {
}

Status OrderedAxisQuery::add_label_range(
    const void* start, const void* end, const void* stride) {
  if (stride != nullptr)
    return Status_AxisQueryError(
        "Cannot add range; Setting label range stride is currently "
        "unsupported.");
  if (range_query_ != nullptr)
    return Status_AxisQueryError(
        "Cannot add range; Setting more than one label range is currently "
        "unsupported.");
  range_query_ = tdb_unique_ptr<RangeQuery>(
      tdb_new(RangeQuery, axis_.get(), storage_manager_, start, end));
  return Status::Ok();
}

Status OrderedAxisQuery::add_label_range_var(
    const void*, uint64_t, const void*, uint64_t) {
  return Status_AxisQueryError(
      "Adding variable length ranges in not yet supported for ordered "
      "labels.");
}

Status OrderedAxisQuery::cancel() {
  if (range_query_)
    RETURN_NOT_OK(range_query_->cancel());
  if (indexed_array_query_)
    RETURN_NOT_OK(indexed_array_query_->cancel());
  if (labelled_array_query_)
    RETURN_NOT_OK(labelled_array_query_->cancel());
  return Status::Ok();
}

Status OrderedAxisQuery::create_data_query() {
  if (indexed_array_query_)
    return Status_AxisQueryError(
        "Cannot create data query. Query already exists.");
  indexed_array_query_ = tdb_unique_ptr<Query>(
      tdb_new(Query, storage_manager_, axis_->indexed_array()));
  return indexed_array_query_->set_layout(Layout::ROW_MAJOR);
}

Status OrderedAxisQuery::finalize() {
  if (range_query_)
    RETURN_NOT_OK(range_query_->finalize());
  if (indexed_array_query_)
    RETURN_NOT_OK(indexed_array_query_->finalize());
  if (labelled_array_query_)
    RETURN_NOT_OK(labelled_array_query_->finalize());
  return Status::Ok();
}

tuple<Status, Range> OrderedAxisQuery::get_index_range() const {
  if (!range_query_)
    return {Status_AxisQueryError("No label range set."), Range()};
  Range range{range_query_->index_range()};
  return {(range_query_->status() == QueryStatus::COMPLETED) ?
              Status::Ok() :
              Status_AxisQueryError("Label query incomplete"),
          range};
}

Status OrderedAxisQuery::resolve_labels() {
  if (range_query_) {
    RETURN_NOT_OK(range_query_->submit());
    RETURN_NOT_OK(range_query_->finalize());
  }
  return Status::Ok();
}

Status OrderedAxisQuery::set_index_ranges(const std::vector<Range>& ranges) {
  if (!indexed_array_query_)
    return Status_AxisQueryError(
        "Cannot set subarray. Data query not initialized.");
  Subarray subarray{
      axis_->indexed_array().get(), Layout::ROW_MAJOR, stats_, logger_};
  RETURN_NOT_OK(subarray.set_ranges_for_dim(0, ranges));
  return indexed_array_query_->set_subarray(subarray);
}

Status OrderedAxisQuery::set_label_data_buffer(
    void* const buffer,
    uint64_t* const buffer_size,
    const bool check_null_buffers) {
  return indexed_array_query_->set_data_buffer(
      axis_->label_attribute()->name(),
      buffer,
      buffer_size,
      check_null_buffers);
}

QueryStatus OrderedAxisQuery::status_data_query() const {
  // TODO: Fix this to have a consistent status.
  if (!labelled_array_query_ && !indexed_array_query_)
    return QueryStatus::COMPLETED;
  if (!labelled_array_query_ && indexed_array_query_)
    return indexed_array_query_->status();
  if (!indexed_array_query_ && labelled_array_query_)
    return labelled_array_query_->status();
  auto labelled_status = labelled_array_query_->status();
  auto indexed_status = indexed_array_query_->status();
  if (labelled_status == QueryStatus::FAILED ||
      indexed_status == labelled_status)
    return labelled_status;
  return indexed_status;
}

QueryStatus OrderedAxisQuery::status_resolve_labels() const {
  if (!range_query_)  // TODO: Fix to be for range  reader
    return QueryStatus::COMPLETED;
  return range_query_->status();
}

Status OrderedAxisQuery::submit_data_query() {
  if (range_query_ && range_query_->status() != QueryStatus::COMPLETED)
    return Status_AxisQueryError(
        "Cannot set data queries until label query completes");
  if (labelled_array_query_)
    RETURN_NOT_OK(labelled_array_query_->submit());
  if (indexed_array_query_) {
    RETURN_NOT_OK(indexed_array_query_->submit());
  }
  return Status::Ok();
}

}  // namespace tiledb::sm
