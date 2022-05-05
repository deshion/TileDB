/**
 * @file tiledb/sm/label_schema/test/unit_label.cc
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
 * Unit tests for labels
 */

#include <catch.hpp>
#include "tiledb/sm/enums/datatype.h"
#include "tiledb/sm/filesystem/uri.h"
#include "tiledb/sm/label_schema/labels.h"

using namespace tiledb::sm;
using namespace tiledb::common;

TEST_CASE("Test adding and accessing label schemas", "[array_labels]") {
  ArrayLabels labels{3};
  // Add 2 labels to the first dimension and 3 labels to third dimension.
  // - Label 1
  auto status = labels.add_external_label(
      0, "label1", Datatype::UINT64, 1, URI("path1"), true, "dim1", "attr1");
  CHECK(status.ok());
  CHECK(labels.label_num(0) == 1);
  CHECK(labels.label_num(1) == 0);
  CHECK(labels.label_num(2) == 0);
  REQUIRE(labels.label_num() == 1);
  // - Label 2
  status = labels.add_external_label(
      2, "label2", Datatype::UINT64, 1, URI("path2"), true, "dim1", "attr1");
  CHECK(status.ok());
  CHECK(labels.label_num(0) == 1);
  CHECK(labels.label_num(1) == 0);
  CHECK(labels.label_num(2) == 1);
  REQUIRE(labels.label_num() == 2);
  // - Label 3
  status = labels.add_external_label(
      0, "label3", Datatype::UINT64, 1, URI("path3"), true, "dim1", "attr2");
  CHECK(status.ok());
  CHECK(labels.label_num(0) == 2);
  CHECK(labels.label_num(1) == 0);
  CHECK(labels.label_num(2) == 1);
  REQUIRE(labels.label_num() == 3);
  // - Label 4
  status = labels.add_external_label(
      2, "label4", Datatype::UINT64, 1, URI("path4"), true, "dim1", "attr1");
  CHECK(status.ok());
  CHECK(labels.label_num(0) == 2);
  CHECK(labels.label_num(1) == 0);
  CHECK(labels.label_num(2) == 2);
  REQUIRE(labels.label_num() == 4);
  // - Label 5
  status = labels.add_external_label(
      2, "label5", Datatype::UINT64, 1, URI("path5"), true, "dim1", "attr1");
  CHECK(status.ok());
  CHECK(labels.label_num(0) == 2);
  CHECK(labels.label_num(1) == 0);
  CHECK(labels.label_num(2) == 3);
  REQUIRE(labels.label_num() == 5);
  // Test unable to add label with repeated name.
  status = labels.add_external_label(
      2, "label2", Datatype::UINT64, 1, URI("path6"), true, "dim1", "attr1");
  CHECK(!status.ok());
  CHECK(labels.label_num(0) == 2);
  CHECK(labels.label_num(1) == 0);
  CHECK(labels.label_num(2) == 3);
  REQUIRE(labels.label_num() == 5);
  // Test unable to add label with dimension larger than max dim
  status = labels.add_external_label(
      3, "label6", Datatype::UINT64, 1, URI("path6"), true, "dim1", "attr1");
  CHECK(!status.ok());
  CHECK(labels.label_num(0) == 2);
  CHECK(labels.label_num(1) == 0);
  CHECK(labels.label_num(2) == 3);
  REQUIRE(labels.label_num() == 5);
  // Test get schema by index
  auto label_schema4 = labels.label_schema(2, 1);
  REQUIRE(label_schema4 != nullptr);
  REQUIRE(label_schema4->name() == std::string("label4"));
  // Test get schema by name - valid
  auto label_schema3 = labels.label_schema_by_name("label3");
  REQUIRE(label_schema3 != nullptr);
  REQUIRE(label_schema3->name() == std::string("label3"));
  // Test get schema by name - invalid
  auto label_schema_none = labels.label_schema_by_name("label6");
  REQUIRE(label_schema_none == nullptr);
  // Test get schema by name and dim id - valid
  auto label_schema1 = labels.label_schema_by_name(0, "label1");
  REQUIRE(label_schema1 != nullptr);
  REQUIRE(label_schema1->name() == std::string("label1"));
  // Test get schema by name and dim id - invalid
  auto label_schema_none2 = labels.label_schema_by_name(1, "label2");
  REQUIRE(label_schema_none2 == nullptr);
}
