/**
 * @file tiledb/sm/array_schema/test/unit_dimension_label.cc
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
 * This file tests the dimension label
 */

#include <catch.hpp>
#include "tiledb/sm/array_schema/dimension_label.h"
#include "tiledb/sm/buffer/buffer.h"
#include "tiledb/sm/enums/label_order.h"

using namespace tiledb::common;
using namespace tiledb::sm;

TEST_CASE(
    "Roundtrip dimension label serialization",
    "[dimension_label][serialize][deserialize]") {
  const uint32_t version{14};
  DimensionLabel::dimension_size_type dim_id{0};
  std::string name{"label0"};
  LabelOrder label_order{LabelOrder::FORWARD};
  bool is_external{false};
  URI indexed_array_uri{"__label/l0/indexed", false};
  URI labelled_array_uri{"__index/l0/labelled", false};
  DimensionLabel::attribute_size_type label_attr_id{1};
  DimensionLabel::attribute_size_type index_attr_id{2};
  DimensionLabel label{dim_id,
                       name,
                       label_order,
                       is_external,
                       indexed_array_uri,
                       labelled_array_uri,
                       label_attr_id,
                       index_attr_id};
  Buffer buffer{};
  auto status1 = label.serialize(&buffer, version);
  if (!status1.ok())
    INFO("Status serialize dimension label: " + status1.to_string());
  REQUIRE(status1.ok());
  ConstBuffer buffer2{&buffer};
  auto&& [status2, label2] = DimensionLabel::deserialize(&buffer2, version);
  if (!status2.ok())
    INFO("Status deserialize dimension label: " + status2.to_string());
  REQUIRE(status2.ok());
  CHECK(dim_id == label2->dimension_id());
  CHECK(name == label2->name());
  CHECK(is_external == label2->is_external());
  INFO("Label URI: " + label2->labelled_array_uri().to_string());
  CHECK(
      labelled_array_uri.to_string() ==
      label2->labelled_array_uri().to_string());
  CHECK(
      indexed_array_uri.to_string() == label2->indexed_array_uri().to_string());
  CHECK(label_attr_id == label2->label_attribute_id());
  CHECK(index_attr_id == label2->index_attribute_id());
}
