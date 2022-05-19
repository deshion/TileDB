/**
 * @file tiledb/sm/array_schema/dimension_label.cc
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
 */

#include "tiledb/sm/array_schema/dimension_label.h"
#include "tiledb/common/common.h"
#include "tiledb/sm/buffer/buffer.h"
#include "tiledb/sm/misc/constants.h"

using namespace tiledb::common;

namespace tiledb::sm {
DimensionLabel::DimensionLabel(
    dimension_size_type dim_id,
    const std::string& name,
    LabelOrder label_order,
    bool is_external,
    const URI& indexed_array_uri,
    const URI& labelled_array_uri,
    attribute_size_type label_attr_id,
    attribute_size_type index_attr_id)
    : dim_id_(dim_id)
    , name_(name)
    , label_order_(label_order)
    , is_external_(is_external)
    , indexed_array_uri_(indexed_array_uri)
    , labelled_array_uri_(labelled_array_uri)
    , label_attr_id_(label_attr_id)
    , index_attr_id_(index_attr_id) {
}

// FORMAT:
//| Field                     | Type       |
//| ------------------------- | ---------- |
//| Dimension ID              | `uint32_t` |
//| Label order               | `uint32_t` |
//| Label attribute ID        | `uint32_t` |
//| Index attribute ID        | `uint32_t` |
//| Is external               | `bool`     |
//| Name length               | `uint64_t` |
//| Indexed array uri length  | `uint64_t` |
//| Labelled array uri length | `uint64_t` |
//| Name                      | `char []`  |
//| Indexed array uri         | `char []`  |
//| Labelled  array uri       | `char []`  |
tuple<Status, shared_ptr<DimensionLabel>> DimensionLabel::deserialize(
    ConstBuffer* buff, uint32_t) {
  // Read dimension ID
  dimension_size_type dim_id;
  RETURN_NOT_OK_TUPLE(
      buff->read(&dim_id, sizeof(dimension_size_type)), nullptr);
  // Read label order
  uint8_t label_order_int;
  RETURN_NOT_OK_TUPLE(buff->read(&label_order_int, sizeof(uint8_t)), nullptr);
  LabelOrder label_order{static_cast<LabelOrder>(label_order_int)};
  // Read label attribute ID
  attribute_size_type label_attr_id;
  RETURN_NOT_OK_TUPLE(
      buff->read(&label_attr_id, sizeof(label_attr_id)), nullptr);
  // Read index attribute ID
  attribute_size_type index_attr_id;
  RETURN_NOT_OK_TUPLE(
      buff->read(&index_attr_id, sizeof(index_attr_id)), nullptr);
  // Read if the label is internal or external
  bool is_external;
  RETURN_NOT_OK_TUPLE(buff->read(&is_external, sizeof(bool)), nullptr);
  // Read the length of the name string
  uint64_t name_size;
  RETURN_NOT_OK_TUPLE(buff->read(&name_size, sizeof(uint64_t)), nullptr);
  // Read the length of the indexed array URI
  uint64_t indexed_array_uri_size;
  RETURN_NOT_OK_TUPLE(
      buff->read(&indexed_array_uri_size, sizeof(uint64_t)), nullptr);
  // Read the length of the labelled array URI
  uint64_t labelled_array_uri_size;
  RETURN_NOT_OK_TUPLE(
      buff->read(&labelled_array_uri_size, sizeof(uint64_t)), nullptr);
  // Read label name
  std::string name;
  name.resize(name_size);
  RETURN_NOT_OK_TUPLE(buff->read(&name[0], name_size), nullptr);
  // Read indexed array URI
  std::string indexed_array_uri;
  indexed_array_uri.resize(indexed_array_uri_size);
  RETURN_NOT_OK_TUPLE(
      buff->read(&indexed_array_uri[0], indexed_array_uri_size), nullptr);
  // READ labelled array URI
  std::string labelled_array_uri;
  labelled_array_uri.resize(labelled_array_uri_size);
  RETURN_NOT_OK_TUPLE(
      buff->read(&labelled_array_uri[0], labelled_array_uri_size), nullptr);
  return {Status::Ok(),
          make_shared<DimensionLabel>(
              HERE(),
              dim_id,
              name,
              label_order,
              is_external,
              URI(indexed_array_uri, is_external),
              URI(labelled_array_uri, is_external),
              label_attr_id,
              index_attr_id)};
}

void DimensionLabel::dump(FILE* out) const {
  if (out == nullptr)
    out = stdout;
  fprintf(
      out,
      "DimensionLabel: Dimension=%i, Name=%s, Order=%s",
      dim_id_,
      name_.c_str(),
      label_order_str(label_order_).c_str());
}

// FORMAT:
//| Field                     | Type       |
//| ------------------------- | ---------- |
//| Dimension ID              | `uint32_t` |
//| Label order               | `uint32_t` |
//| Label attribute ID        | `uint32_t` |
//| Index attribute ID        | `uint32_t` |
//| Is external               | `bool`     |
//| Name length               | `uint64_t` |
//| Indexed array uri length  | `uint64_t` |
//| Labelled array uri length | `uint64_t` |
//| Name                      | `char []`  |
//| Indexed array uri         | `char []`  |
//| Labelled  array uri       | `char []`  |
Status DimensionLabel::serialize(Buffer* buff, uint32_t) const {
  RETURN_NOT_OK(buff->write(&dim_id_, sizeof(dimension_size_type)));
  uint8_t label_type_int{static_cast<uint8_t>(label_order_)};
  RETURN_NOT_OK(buff->write(&label_type_int, sizeof(uint8_t)));
  RETURN_NOT_OK(buff->write(&label_attr_id_, sizeof(attribute_size_type)));
  RETURN_NOT_OK(buff->write(&index_attr_id_, sizeof(attribute_size_type)));
  RETURN_NOT_OK(buff->write(&is_external_, sizeof(bool)));
  uint64_t name_size{name_.size()};
  RETURN_NOT_OK(buff->write(&name_size, sizeof(uint64_t)));
  uint64_t indexed_array_uri_size{indexed_array_uri_.to_string().size()};
  RETURN_NOT_OK(buff->write(&indexed_array_uri_size, sizeof(uint64_t)));
  uint64_t labelled_array_uri_size{labelled_array_uri_.to_string().size()};
  RETURN_NOT_OK(buff->write(&labelled_array_uri_size, sizeof(uint64_t)));
  RETURN_NOT_OK(buff->write(name_.c_str(), name_size));
  RETURN_NOT_OK(
      buff->write(indexed_array_uri_.c_str(), indexed_array_uri_size));
  RETURN_NOT_OK(
      buff->write(labelled_array_uri_.c_str(), labelled_array_uri_size));
  return Status::Ok();
}

}  // namespace tiledb::sm
