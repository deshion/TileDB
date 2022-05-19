/**
 * @file tiledb/sm/array_schema/dimension_label.h
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
 * Defines the dimension label class
 */

#ifndef TILEDB_DIMENSION_LABEL_H
#define TILEDB_DIMENSION_LABEL_H

#include "tiledb/common/common.h"
#include "tiledb/sm/enums/label_order.h"
#include "tiledb/sm/filesystem/uri.h"

using namespace tiledb::common;

namespace tiledb::sm {

class Buffer;
class ConstBuffer;

class DimensionLabel {
 public:
  /** TODO: Add docs */
  using dimension_size_type = uint32_t;

  /** TODO: Add docs */
  using attribute_size_type = uint32_t;

  /** Default constructor is not C.41. */
  DimensionLabel() = delete;

  /** TODO: Add docs */
  DimensionLabel(
      dimension_size_type dim_id,
      const std::string& name,
      LabelOrder label_order,
      bool is_external,
      const URI& indexed_array_uri,
      const URI& labelled_array_uri,
      attribute_size_type label_attr_id,
      attribute_size_type index_attr_id);

  /**
   * Populates the object members from the data in the input binary buffer.
   *
   * @param buff The buffer to deserialize from.
   * @param version The array schema version.
   * @return Status and DimensionLabel
   */
  static tuple<Status, shared_ptr<DimensionLabel>> deserialize(
      ConstBuffer* buff, uint32_t version);

  /**
   * Dumps the dimension label contents in ASCII form in the selected output.
   */
  void dump(FILE* out) const;

  /** TODO: Add docs */
  inline dimension_size_type dimension_id() const {
    return dim_id_;
  }

  inline attribute_size_type index_attribute_id() const {
    return index_attr_id_;
  }

  inline const URI& indexed_array_uri() const {
    return indexed_array_uri_;
  }

  /** TODO: Add docs. */
  inline bool is_external() const {
    return is_external_;
  }

  inline attribute_size_type label_attribute_id() const {
    return label_attr_id_;
  }

  /** TODO: Add docs */
  inline LabelOrder label_order() const {
    return label_order_;
  }

  inline const URI& labelled_array_uri() const {
    return labelled_array_uri_;
  }

  /** TODO: Add docs */
  inline const std::string& name() const {
    return name_;
  }

  /**
   * Serializes the dimension label object into a buffer.
   *
   * @param buff The buffer the array schema is serialized into.
   * @param version The array schema version.
   * @return Status
   */
  Status serialize(Buffer* buff, uint32_t version) const;

 private:
  /** TODO: Add docs */
  dimension_size_type dim_id_;

  /** TODO: Add docs */
  std::string name_;

  /** TODO: Add docs */
  LabelOrder label_order_;

  /** TODO: Add docs */
  bool is_external_;

  /** TODO: Add docs */
  URI indexed_array_uri_;

  /** TODO: Add docs */
  URI labelled_array_uri_;

  /** TODO: Add docs */
  attribute_size_type label_attr_id_;

  /** TODO: Add docs */
  attribute_size_type index_attr_id_;
};

}  // namespace tiledb::sm

#endif
