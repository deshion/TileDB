/**
 * @file tiledb/sm/label_schema/label.h
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
 * Defines an object that stores the location of a dimension label
 */

#ifndef TILEDB_LABEL_H
#define TILEDB_LABEL_H

#include <vector>
#include "tiledb/common/common.h"
#include "tiledb/sm/filesystem/uri.h"

using namespace tiledb::common;

namespace tiledb::sm {

enum class Datatype : uint8_t;

/** Return a Status_ArrayLabelsError error class Status with a given message **/
inline Status Status_ArrayLabelsError(const std::string& msg) {
  return {"[TileDB::ArrayLabels] Error", msg};
}

/** Interface for label schemas */
class LabelSchema {
 public:
  virtual ~LabelSchema() = default;

  virtual uint32_t cell_val_num() const = 0;

  virtual const std::string& name() const = 0;

  virtual bool relative_uri() const = 0;

  virtual Datatype type() const = 0;

  virtual const URI& uri() const = 0;
};

/** Schema for accessing a dimension label not managaed by this array. */
class ExternalLabelSchema : public LabelSchema {
 public:
  ExternalLabelSchema() = delete;

  /**
   * Constructor.
   *
   * TODO: define parameters
   */
  ExternalLabelSchema(
      const std::string& name,
      Datatype type,
      uint32_t cell_val_num,
      const URI& uri,
      bool relative_uri,
      const std::string& index_name,
      const std::string& label_name);

  inline uint32_t cell_val_num() const override {
    return cell_val_num_;
  }

  inline const std::string& name() const override {
    return name_;
  }

  inline bool relative_uri() const override {
    return relative_uri_;
  }

  inline Datatype type() const override {
    return type_;
  }

  inline const URI& uri() const override {
    return uri_;
  }

 private:
  /** Name of the label. */
  std::string name_;

  /** The datatype of the label. */
  Datatype type_;

  /** The number of values per cell for the label. */
  uint32_t cell_val_num_;

  /** The URI of the object to use a dimension label. */
  URI uri_;

  /** If the URI path is relative to the array schema. */
  bool relative_uri_;

  /**
   * The name of the component of the object at the URI to use as an index.
   */
  std::string index_name_;

  /**
   * The name of the component of the object at the URI to use as a label.
   **/
  std::string label_name_;
};

class ArrayLabels {
 public:
  /** Constructor is not C.41 compliant. */
  ArrayLabels() = delete;

  /** Constructor
   *
   * TODO: Add parameters
   **/
  ArrayLabels(const uint32_t dim_num);

  /**
   * Constructor
   *
   * TODO: Add parameters
   */
  ArrayLabels(std::vector<std::vector<shared_ptr<LabelSchema>>> labels);

  Status add_external_label(
      uint32_t dimension_index,
      const std::string& name,
      Datatype type,
      uint32_t cell_val_num,
      const URI& uri,
      bool relative_uri,
      const std::string& index_name,
      const std::string& label_name);

  /**
   * Returns the label at the provided index.
   *
   * This method looks up the label without performaing any indexing checks.
   *
   * @param dim_index The index of the dimension the label is defined on
   * @param label_index The index of the label
   * @returns Label
   */
  shared_ptr<const LabelSchema> label_schema(
      const uint32_t dim_index, const uint32_t label_index) const {
    return labels_by_dim_index_[dim_index][label_index];
  }

  /**
   * Returns the label with the requested name. Returns a nullptr if the label
   * wasn't found.
   *
   * @param label_name Name of the label
   * @returns Label or nullptr
   */
  shared_ptr<const LabelSchema> label_schema_by_name(
      const std::string& label_name) const;

  /**
   * Returns Dthe label at the provided index.
   *
   * This method will verify the label with the name is on the requested
   * dimension.
   *
   * @param dim_index The index of the dimension the label is defined on
   * @param label_name Name of the label
   * @returns Label or nullptr
   */
  shared_ptr<const LabelSchema> label_schema_by_name(
      const uint32_t dim_index, const std::string& label_name) const;

  /**
   * Get the number of labels.
   *
   * @returns Total number of labels.
   */
  inline uint64_t label_num() const {
    // Check if this should be uint64_t to be consistent
    return labels_by_name_.size();
  }

  /**
   * Get the number of labels on a specific dimension.
   *
   * @param dim_index The index of the dimension to return the labels for
   * @returns Total number of labels for a specific label.
   */
  inline uint64_t label_num(const uint32_t dim_index) const {
    return labels_by_dim_index_[dim_index].size();
  }

 private:
  /**
   * Map for accesing labels by dimension-based indexing.
   *
   * The component [i, j] is the jth label added to the ith dimension.
   **/
  std::vector<std::vector<shared_ptr<LabelSchema>>> labels_by_dim_index_;

  /**
   * Map for accessing labels by name.
   */
  std::unordered_map<std::string, tuple<uint32_t, shared_ptr<LabelSchema>>>
      labels_by_name_;
};

}  // namespace tiledb::sm

#endif
