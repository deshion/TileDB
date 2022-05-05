/**
 * @file tiledb/sm/label_schema/labels.cc
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

#include "tiledb/sm/label_schema/labels.h"
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
    const std::string& name,
    Datatype type,
    uint32_t cell_val_num,
    const URI& uri,
    bool relative_uri,
    const std::string& index_name,
    const std::string& label_name)
    : name_(name)
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
    : labels_by_dim_index_{dim_num}
    , labels_by_name_{} {
}

ArrayLabels::ArrayLabels(
    std::vector<std::vector<shared_ptr<LabelSchema>>> labels)
    : labels_by_dim_index_{labels}
    , labels_by_name_{} {
  if (labels_by_dim_index_.size() == 0)
    throw std::invalid_argument(
        "Cannot add labels to an array with no dimensions.");
  size_t nlabels{0};
  for (uint32_t dim_idx{0}; dim_idx < labels_by_dim_index_.size(); ++dim_idx) {
    nlabels += labels_by_dim_index_[dim_idx].size();
    for (auto& label : labels_by_dim_index_[dim_idx]) {
      labels_by_name_.emplace(label->name(), std::tie(dim_idx, label));
    }
  }
  if (labels_by_name_.size() != nlabels)
    throw std::invalid_argument("Array label names must be unique.");
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
  labels_by_dim_index_[dimension_index].emplace_back(
      make_shared<ExternalLabelSchema>(
          HERE(),
          name,
          type,
          cell_val_num,
          uri,
          relative_uri,
          index_name,
          label_name));
  labels_by_name_.emplace(
      name,
      std::tie(dimension_index, labels_by_dim_index_[dimension_index].back()));
  return Status::Ok();
}

shared_ptr<const LabelSchema> ArrayLabels::label_schema_by_name(
    const std::string& label_name) const {
  auto it = labels_by_name_.find(label_name);
  if (it == labels_by_name_.end())
    return nullptr;
  return std::get<1>(it->second);
}

shared_ptr<const LabelSchema> ArrayLabels::label_schema_by_name(
    const uint32_t dim_index, const std::string& label_name) const {
  auto it = labels_by_name_.find(label_name);
  if (it == labels_by_name_.end())
    return nullptr;
  auto member = it->second;
  if (std::get<0>(it->second) != dim_index)
    return nullptr;
  return std::get<1>(it->second);
}

}  // namespace tiledb::sm
