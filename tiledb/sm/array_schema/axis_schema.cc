/**
 * @file tiledb/sm/axis/axis_schema.cc
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

#include "tiledb/sm/array_schema/axis_schema.h"
#include "tiledb/common/common.h"
#include "tiledb/sm/array_schema/array_schema.h"
#include "tiledb/sm/array_schema/attribute.h"
#include "tiledb/sm/array_schema/dimension.h"
#include "tiledb/sm/array_schema/domain.h"
#include "tiledb/sm/enums/array_type.h"
#include "tiledb/sm/enums/layout.h"
#include "tiledb/sm/misc/types.h"
#include "tiledb/type/range/range.h"

using namespace tiledb::common;
using namespace tiledb::type;

namespace tiledb::sm {

/******************************************/
/*             AxisComponent              */
/******************************************/

AxisComponent::AxisComponent(const std::string& name, Datatype type)
    : AxisComponent(
          name,
          type,
          (datatype_is_string(type)) ? constants::var_num : 1,
          Range(),
          ByteVecValue(),
          ByteVecValue(),
          FilterPipeline(),
          FilterPipeline()) {
}

AxisComponent::AxisComponent(
    const std::string name,
    Datatype type,
    uint32_t cell_val_num,
    const Range& domain,
    ByteVecValue tile_extent,
    ByteVecValue fill_value,
    const FilterPipeline& attr_filters,
    const FilterPipeline& dim_filters)
    : dimension_(make_shared<Dimension>(
          HERE(), name, type, cell_val_num, domain, dim_filters, tile_extent))
    , attribute_(make_shared<Attribute>(
          HERE(),
          name,
          type,
          false,
          cell_val_num,
          attr_filters,
          fill_value,
          0)) {
}

AxisComponent::AxisComponent(
    shared_ptr<Dimension> dim, shared_ptr<Attribute> attr)
    : dimension_(dim)
    , attribute_(attr) {
  auto&& [is_ok, msg] = is_compatible(dim.get(), attr.get());
  if (!is_ok)
    throw std::invalid_argument(
        "Cannot construct axis component; " + msg.value());
}

tuple<bool, optional<std::string>> AxisComponent::is_compatible(
    const Dimension* dim, const Attribute* attr) {
  if (attr->nullable())
    return {false, "Attribute cannot be nullable."};
  if (dim->type() != attr->type())
    return {false, "Attribute and dimension datatype do not match."};
  if (dim->cell_val_num() != attr->cell_val_num())
    return {false,
            "Attribute and dimension number of values per cell do not match"};
  return {true, nullopt};
}
/******************************************/
/*              AxisSchema                */
/******************************************/

AxisSchema::AxisSchema(
    LabelOrder label_order,
    shared_ptr<const AxisComponent> index_component,
    shared_ptr<const AxisComponent> label_component,
    uint64_t capacity,
    Layout cell_order,
    Layout tile_order)
    : label_order_(label_order)
    , indexed_array_schema_(make_shared<ArraySchema>(HERE(), ArrayType::DENSE))
    , labelled_array_schema_(
          make_shared<ArraySchema>(HERE(), ArrayType::SPARSE))
    , label_attr_id_(0)
    , index_attr_id_(0) {
  // Set-up indexed array
  std::vector<shared_ptr<Dimension>> index_dims{index_component->dimension()};
  throw_if_not_ok(indexed_array_schema_->set_domain(
      make_shared<Domain>(HERE(), cell_order, index_dims, tile_order)));
  throw_if_not_ok(
      indexed_array_schema_->add_attribute(label_component->attribute()));
  indexed_array_schema_->set_capacity(capacity);
  auto status = indexed_array_schema_->check();
  if (!status.ok())
    throw StatusException(Status_AxisSchemaError(
        "Index array schema check failed; Index array schema is not valid."));
  // Set-up labelled array
  std::vector<shared_ptr<Dimension>> label_dims{label_component->dimension()};
  throw_if_not_ok(labelled_array_schema_->set_domain(make_shared<Domain>(
      HERE(), Layout::ROW_MAJOR, label_dims, Layout::ROW_MAJOR)));
  throw_if_not_ok(
      labelled_array_schema_->add_attribute(index_component->attribute()));
  labelled_array_schema_->set_capacity(capacity);
  status = labelled_array_schema_->check();
  if (!status.ok())
    throw StatusException(Status_AxisSchemaError(
        "Label array schema check failed; Label array schema is not valid."));
}

AxisSchema::AxisSchema(
    LabelOrder label_order,
    shared_ptr<ArraySchema> indexed_array_schema,
    shared_ptr<ArraySchema> labelled_array_schema,
    const attribute_size_type label_attr_id,
    const attribute_size_type index_attr_id)
    : label_order_(label_order)
    , indexed_array_schema_(indexed_array_schema)
    , labelled_array_schema_(labelled_array_schema)
    , label_attr_id_(label_attr_id)
    , index_attr_id_(index_attr_id) {
  // Check arrays are one dimensional
  if (labelled_array_schema->dim_num() != 1)
    throw std::invalid_argument(
        "Invalid axis schema; Labelled array must be one dimensional");
  if (indexed_array_schema->dim_num() != 1)
    throw std::invalid_argument(
        "Invalid axis schema; Indexed array must be one dimensional");
  // Check index and label attribute exist
  if (label_attr_id_ >= indexed_array_schema->attribute_num())
    throw std::invalid_argument(
        "Invalid axis schema; No label attribute " +
        std::to_string(index_attr_id_));
  if (index_attr_id_ >= labelled_array_schema->attribute_num())
    throw std::invalid_argument(
        "Invalid axis schema; No index attribute " +
        std::to_string(label_attr_id_));
  // Check the types are consistent between the two arrays
  auto [is_ok, msg] = AxisComponent::is_compatible(
      labelled_array_schema_->dimension_ptr(0),
      indexed_array_schema_->attribute(label_attr_id_));
  if (!is_ok)
    throw std::invalid_argument(
        "Invalid axis schema; Incompatible definitions of the label "
        "dimension and label attribute. " +
        msg.value());
  std::tie(is_ok, msg) = AxisComponent::is_compatible(
      indexed_array_schema_->dimension_ptr(0),
      labelled_array_schema_->attribute(index_attr_id_));
  if (!is_ok)
    throw std::invalid_argument(
        "Invalid axis schema; Incompatible definitions of the index "
        "dimension and index attribute. " +
        msg.value());
}

const Attribute* AxisSchema::index_attribute() const {
  return labelled_array_schema_->attribute(index_attr_id_);
}

const Dimension* AxisSchema::index_dimension() const {
  return indexed_array_schema_->dimension_ptr(0);
}

bool AxisSchema::is_compatible_label(const Dimension* dim) const {
  auto dim0 = index_dimension();
  return dim->type() == dim0->type() &&
         dim->cell_val_num() == dim0->cell_val_num() &&
         dim->domain() == dim0->domain();
}

const Attribute* AxisSchema::label_attribute() const {
  return indexed_array_schema_->attribute(label_attr_id_);
}

const Dimension* AxisSchema::label_dimension() const {
  return labelled_array_schema_->dimension_ptr(0);
}

}  // namespace tiledb::sm
