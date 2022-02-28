/**
 * @file tiledb/sm/array_schema/label.cc
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

#include "tiledb/sm/array_schema/labels.h"
#include "tiledb/common/common.h"
#include "tiledb/sm/array_schema/domain.h"
#include "tiledb/sm/enums/datatype.h"
#include "tiledb/sm/filesystem/uri.h"

using namespace tiledb::common;

namespace tiledb::sm {

/***********************/
/* ExternalLabelSchema */
/***********************/

ExternalLabelSchema::ExternalLabelSchema(
    uint32_t dimension_index,
    const std::string& name,
    Datatype type,
    uint32_t cell_val_num,
    const URI& uri,
    bool relative_uri,
    const std::string& index_name,
    const std::string& label_name)
    : dimension_index_(dimension_index)
    , name_(name)
    , type_(type)
    , cell_val_num_(cell_val_num)
    , uri_(uri)
    , relative_uri_(relative_uri)
    , index_name_(index_name)
    , label_name_(label_name) {
}

/***************/
/* ArrayLabels */
/***************/

ArrayLabels::ArrayLabels(const uint32_t dim_num)
    : labels_{}
    , labels_by_dim_index_{dim_num}
    , labels_by_name_{} {
}

ArrayLabels::ArrayLabels(
    const uint32_t dim_num, std::vector<shared_ptr<LabelSchema>> labels)
    : labels_{labels}
    , labels_by_dim_index_{dim_num}
    , labels_by_name_{} {
  for (auto& label : labels) {
    const auto index = label->dimension_index();
    if (index >= dim_num)
      throw std::invalid_argument(
          "Cannot have a label to dimension " + std::to_string(index) +
          " on an array with " + std::to_string(dim_num) + " dimensions.");
    labels_by_dim_index_[index].emplace_back(label);
    labels_by_name_.emplace(label->name(), label);
    if (labels_by_name_.size() != labels_.size())
      throw std::invalid_argument("Array label names must be unique.");
  }
}

Status ArrayLabels::add_external_label(
    uint32_t dimension_index,
    const std::string& name,
    Datatype type,
    uint32_t cell_val_num,
    const URI& uri,
    bool relative_uri,
    const std::string& index_name,
    const std::string& label_name) {
  if (dimension_index >= labels_by_dim_index_.size())
    return Status_ArrayLabelsError(
        "Cannot add label for dimension " + std::to_string(dimension_index) +
        " to an array with " + std::to_string(labels_by_dim_index_.size()) +
        "dimensions.");
  if (labels_by_name_.find(name) != labels_by_name_.end())
    return Status_ArrayLabelsError(
        "Cannot add label " + name + " to array, label already exists.");
  labels_.emplace_back(make_shared<ExternalLabelSchema>(
      HERE(),
      dimension_index,
      name,
      type,
      cell_val_num,
      uri,
      relative_uri,
      index_name,
      label_name));
  labels_by_dim_index_[dimension_index].emplace_back(labels_.back());
  labels_by_name_.emplace(name, labels_.back());
  return Status::Ok();
}

}  // namespace tiledb::sm
