/**
 * @file tiledb/sm/axis/axis_schema.h
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
 * @section DESCRIPTION
 *
 * Defines the axis schema class
 */

#ifndef TILEDB_AXIS_SCHEMA_H
#define TILEDB_AXIS_SCHEMA_H

#include "tiledb/common/common.h"
#include "tiledb/sm/enums/label_order.h"
#include "tiledb/sm/enums/layout.h"
#include "tiledb/sm/misc/constants.h"

using namespace tiledb::common;

namespace tiledb::type {
class Range;
}

using namespace tiledb::type;

namespace tiledb::sm {

class ArraySchema;
class Attribute;
class Dimension;
class ByteVecValue;
class FilterPipeline;

/**
 * Return a Status_AxisSchema error class Status with a given message
 **/
inline Status Status_AxisSchemaError(const std::string& msg) {
  return {"[TileDB::AxisSchema] Error", msg};
}

/**
 * Return a Status_AxisComponent error class Status with a given message
 **/
inline Status Status_AxisComponentError(const std::string& msg) {
  return {"[TileDB::AxisComponent] Error", msg};
}

/**
 * A component is a combination dimension and attribute with matching datatypes.
 */
class AxisComponent {
 public:
  /**
   * Default constructor is not C.41 compliant.
   */
  AxisComponent() = delete;

  /**
   * Constructor.
   *
   * @param name The name of the dimension and attribute.
   * @param type The type of the dimension and attribute.
   */
  AxisComponent(const std::string& name, Datatype type);

  /**
   * Constructor
   *
   * @param name The name of the dimension and attribute.
   * @param type The type of the dimension and attribute.
   * @param cell_val_num The number of values in cell.
   * @param domain The domain the defines the interval the dimension is valid
   * on.
   * @param tile_extent The tile extent of the dimension.
   * @param fill_value The default fill value for the attribute.
   * @param attr_filters Compression filters for the attribute
   * @param dim_filters Compression filters for the dimension
   */
  AxisComponent(
      const std::string name,
      Datatype type,
      uint32_t cell_val_num,
      const Range& domain,
      ByteVecValue tile_extent,
      ByteVecValue fill_value,
      const FilterPipeline& attr_filters,
      const FilterPipeline& dim_filters);

  /**
   * Constructor.
   *
   * @param dim Dimension for the axis.
   * @param attr Attribute for the axis.
   */
  AxisComponent(shared_ptr<Dimension> dim, shared_ptr<Attribute> attr);

  /** Returns the attribute of the component. */
  inline shared_ptr<Attribute> attribute() const {
    return attribute_;
  }

  /** Returnst the dimension of the component. */
  inline shared_ptr<Dimension> dimension() const {
    return dimension_;
  }

  /**
   * Returns true if the dimension and attribute has the same datatype, cell
   * value number, and nullability.
   *
   * @param dim Dimension to check
   * @param attr Attribute to check
   */
  static tuple<bool, optional<std::string>> is_compatible(
      const Dimension* dim, const Attribute* attr);

 private:
  shared_ptr<Dimension> dimension_;
  shared_ptr<Attribute> attribute_;
};

/**
 * Schema for an axis. An axis consists of two one-dimensional arrays used to
 * define a dimension label.
 */
class AxisSchema {
 public:
  /**
   * Size type for the number of labels in an axis and for label indices.
   *
   * This must be the same as ArraySchema::attribute_size_type
   */
  using attribute_size_type = unsigned int;

  /** Default constructor is not C.41 compliant. */
  AxisSchema() = delete;

  /**
   * Constructor.
   *
   * @param label_order Order of the labels relative to the index for the axis.
   * @param index_component Definition of the index dimension and attribute.
   * @param label_component Definition of the label dimension and attribute.
   * @param capacity Capactiy for the arrays.
   * @param cell_order The cell order for both arrays.
   * @param tile_order The tile order for both arrays.
   */
  AxisSchema(
      LabelOrder label_order,
      shared_ptr<const AxisComponent> index_component,
      shared_ptr<const AxisComponent> label_component,
      uint64_t capacity = constants::capacity,
      Layout cell_order = Layout::ROW_MAJOR,
      Layout tile_order = Layout::ROW_MAJOR);

  /**
   * Constructor.
   *
   * @param label_order Order of the labels relative to the index for the axis.
   * @param indexed_array_schema Array schema for the array with indices defined
   * on the dimension.
   * @param labelled_array_schema Array schema for the array with labels defined
   * on the dimension.
   * @param label_attr_id The integer ID for the label attribute on the indexed
   * array.
   * @param index_attr_id The integer ID for the index attribute on the labelled
   * array.
   */
  AxisSchema(
      LabelOrder label_order,
      shared_ptr<ArraySchema> indexed_array_schema,
      shared_ptr<ArraySchema> labelled_array_schema,
      const attribute_size_type label_attr_id = 0,
      const attribute_size_type index_attr_id = 0);

  /** Returns the index attribute from the labelled array. */
  inline attribute_size_type index_attribute_id() const {
    return index_attr_id_;
  }

  /** Returns the index attribute from the labelled array. */
  const Attribute* index_attribute() const;

  /** Returns the index dimension from the indexed array. */
  const Dimension* index_dimension() const;

  /**
   * Checks if this axis is compatible as a dimension label for a given
   * dimension.
   *
   * @param dim Dimension to check compatibility against
   * @returns If the axis is compatible as a dimension label
   */
  bool is_compatible_label(const Dimension* dim) const;

  /** Returns the label attribute from the indexed array. */
  const Attribute* label_attribute() const;

  /** Returns the label dimension from the labelled array. */
  const Dimension* label_dimension() const;

  /** Returns the index of the label attribute in the indexed array. */
  inline attribute_size_type label_attribute_id() const {
    return label_attr_id_;
  }

  /** Returns the label order type of this axis. */
  inline LabelOrder label_order() const {
    return label_order_;
  }

 private:
  /** Order of the labels relative to the indices. */
  LabelOrder label_order_;

  /** Schema for the array with indices defined on the dimension. */
  shared_ptr<ArraySchema> indexed_array_schema_;

  /** Schema for the array with labels defined on the dimension. */
  shared_ptr<ArraySchema> labelled_array_schema_;

  /** The integer ID of the label attribute on the indexed array. */
  attribute_size_type label_attr_id_;

  /** The integer ID of the index attribute on the labelled array. */
  attribute_size_type index_attr_id_;
};

}  // namespace tiledb::sm

#endif
