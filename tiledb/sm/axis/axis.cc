/**
 * @file tiledb/sm/axis/axis.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2022 TileDB, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tiledb/sm/axis/axis.h"
#include "tiledb/common/common.h"
#include "tiledb/sm/array/array.h"
#include "tiledb/sm/array_schema/attribute.h"
#include "tiledb/sm/array_schema/axis_schema.h"
#include "tiledb/sm/array_schema/dimension.h"
#include "tiledb/sm/enums/label_order.h"
#include "tiledb/sm/filesystem/uri.h"
#include "tiledb/type/range/range.h"

using namespace tiledb::common;

namespace tiledb::sm {

Axis::Axis(
    const URI& indexed_array_uri,
    const URI& labelled_array_uri,
    StorageManager* storage_manager,
    LabelOrder label_order,
    const AxisSchema::attribute_size_type label_attr_id,
    const AxisSchema::attribute_size_type index_attr_id)
    : indexed_array_(
          make_shared<Array>(HERE(), indexed_array_uri, storage_manager))
    , labelled_array_(
          make_shared<Array>(HERE(), labelled_array_uri, storage_manager))
    , label_order_(label_order)
    , label_attr_id_(label_attr_id)
    , index_attr_id_(index_attr_id)
    , schema_{nullptr} {
}

Status Axis::close() {
  auto status_indexed = indexed_array_->close();
  auto status_labelled = labelled_array_->close();
  if (!status_indexed.ok() || !status_labelled.ok())
    return Status_AxisError("Unable to close both axis arrays.");
  return Status::Ok();
}

const Attribute* Axis::index_attribute() const {
  if (!schema_)
    throw std::logic_error("Axis schema does not exist. Axis must be opened.");
  return schema_->index_attribute();
}

const Dimension* Axis::index_dimension() const {
  if (!schema_)
    throw std::logic_error("Axis schema does not exist. Axis must be opened.");
  return schema_->index_dimension();
}

const Attribute* Axis::label_attribute() const {
  if (!schema_)
    throw std::logic_error("Axis schema does not exist. Axis must be opened.");
  return schema_->label_attribute();
}

const Dimension* Axis::label_dimension() const {
  if (!schema_)
    throw std::logic_error("Axis schema does not exist. Axis must be opened.");
  return schema_->label_dimension();
}

Status Axis::open(
    QueryType query_type,
    EncryptionType encryption_type,
    const void* encryption_key,
    uint32_t key_length) {
  RETURN_NOT_OK(indexed_array_->open(
      query_type, encryption_type, encryption_key, key_length));
  RETURN_NOT_OK(labelled_array_->open(
      query_type, encryption_type, encryption_key, key_length));
  RETURN_NOT_OK(load_schema());
  return Status::Ok();
}

Status Axis::open(
    QueryType query_type,
    uint64_t timestamp_start,
    uint64_t timestamp_end,
    EncryptionType encryption_type,
    const void* encryption_key,
    uint32_t key_length) {
  RETURN_NOT_OK(indexed_array_->open(
      query_type,
      timestamp_start,
      timestamp_end,
      encryption_type,
      encryption_key,
      key_length));
  RETURN_NOT_OK(labelled_array_->open(
      query_type,
      timestamp_start,
      timestamp_end,
      encryption_type,
      encryption_key,
      key_length));
  RETURN_NOT_OK(load_schema());
  return Status::Ok();
}

Status Axis::open_without_fragments(
    EncryptionType encryption_type,
    const void* encryption_key,
    uint32_t key_length) {
  RETURN_NOT_OK(indexed_array_->open_without_fragments(
      encryption_type, encryption_key, key_length));
  RETURN_NOT_OK(labelled_array_->open_without_fragments(
      encryption_type, encryption_key, key_length));
  RETURN_NOT_OK(load_schema());
  return Status::Ok();
}

Status Axis::load_schema() {
  auto&& [label_status, label_schema] = labelled_array_->get_array_schema();
  if (!label_status.ok())
    return label_status;
  auto&& [index_status, index_schema] = indexed_array_->get_array_schema();
  if (!index_status.ok())
    return index_status;
  try {
    schema_ = make_shared<AxisSchema>(
        HERE(),
        label_order_,
        index_schema.value(),
        label_schema.value(),
        label_attr_id_,
        index_attr_id_);
  } catch (std::invalid_argument& except) {
    return Status_AxisError(except.what());
  }
  return Status::Ok();
}

}  // namespace tiledb::sm
