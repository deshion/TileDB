/**
 * @file tiledb/sm/axis/test/unit_array_schema.cc
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
 * This file contains unit tests for the array schema
 */

#include <catch.hpp>

#include "tiledb/sm/array_schema/test/unit_array_schema_helper.h"

using namespace tiledb::common;
using namespace tiledb::sm;

TEST_CASE("Test repeating names", "[array_schema]") {
  SECTION("Label with dimension name okay") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0)};
    auto axis = test::make_axis_schema<uint64_t, double>(
        LabelOrder::FORWARD,
        dims[0].get(),
        16,
        0,
        Datatype::FLOAT64,
        1,
        -1.0,
        1.0,
        0.1,
        0.0);

    auto array_dim = dims[0].get();
    auto label_dim = axis->index_dimension();
    REQUIRE(array_dim->type() == label_dim->type());

    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto status = schema->add_dimension_label(0, "x", axis, false, true);
    REQUIRE(status.ok());
    status = schema->check();
    REQUIRE(status.ok());
  }

  SECTION("Catch shared dimension/attribute name") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("x", Datatype::UINT64, false, 1, 0)};
    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto status = schema->check();
    INFO(status.to_string());
    REQUIRE(!status.ok());
  }

  SECTION("Catch repeating dimension name") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16),
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0)};
    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto status = schema->check();
    INFO(status.to_string());
    REQUIRE(!status.ok());
  }

  SECTION("Catch repeating attribute name") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0),
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0)};
    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto axis = test::make_axis_schema<uint64_t, uint64_t>(
        LabelOrder::FORWARD,
        dims[0].get(),
        16,
        0,
        Datatype::UINT64,
        1,
        16,
        31,
        16,
        0);
    auto dim = dims[0].get();
    auto dim0 = axis->index_dimension();
    auto dim0_data = static_cast<const uint64_t*>(dim0->domain().data());
    auto dim_data = static_cast<const uint64_t*>(dim->domain().data());
    REQUIRE(dim_data[0] == dim0_data[0]);
    REQUIRE(dim_data[1] == dim0_data[1]);
    auto status = schema->check();
    INFO(status.to_string());
    REQUIRE(!status.ok());
  }

  SECTION("Catch repeating label name shared with dim when adding label") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0)};
    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto axis = test::make_axis_schema<uint64_t, uint64_t>(
        LabelOrder::FORWARD,
        dims[0].get(),
        16,
        0,
        Datatype::UINT64,
        1,
        0,
        16,
        16,
        0);
    auto status = schema->add_dimension_label(0, "x", axis, true, true);
    REQUIRE(status.ok());
    status = schema->add_dimension_label(0, "x", axis, true, false);
    INFO(status.to_string());
    REQUIRE(!status.ok());
  }

  SECTION("Catch repeating label name shared with dim with check") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0)};
    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto axis = test::make_axis_schema<uint64_t, uint64_t>(
        LabelOrder::FORWARD,
        dims[0].get(),
        16,
        0,
        Datatype::UINT64,
        1,
        0,
        16,
        16,
        0);
    auto status = schema->add_dimension_label(0, "x", axis, false, true);
    REQUIRE(status.ok());
    status = schema->add_dimension_label(0, "x", axis, false, true);
    REQUIRE(status.ok());
    status = schema->check();
    INFO(status.to_string());
    REQUIRE(!status.ok());
  }

  SECTION("Catch repeating label name not shared with dim when adding label") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0)};
    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto axis = test::make_axis_schema<uint64_t, uint64_t>(
        LabelOrder::FORWARD,
        dims[0].get(),
        16,
        0,
        Datatype::UINT64,
        1,
        0,
        16,
        16,
        0);
    auto status = schema->add_dimension_label(0, "y", axis, true, false);
    REQUIRE(status.ok());
    status = schema->add_dimension_label(0, "y", axis, true, false);
    INFO(status.to_string());
    REQUIRE(!status.ok());
  }

  SECTION("Catch repeating label name not shared with dim with check") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0)};
    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto axis = test::make_axis_schema<uint64_t, uint64_t>(
        LabelOrder::FORWARD,
        dims[0].get(),
        16,
        0,
        Datatype::UINT64,
        1,
        0,
        16,
        16,
        0);
    auto status = schema->add_dimension_label(0, "y", axis, false, false);
    REQUIRE(status.ok());
    status = schema->add_dimension_label(0, "y", axis, false, false);
    REQUIRE(status.ok());
    status = schema->check();
    INFO(status.to_string());
    REQUIRE(!status.ok());
  }

  SECTION("Catch shared label/attribute name when adding label") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0)};
    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto status = schema->check();
    auto axis = test::make_axis_schema<uint64_t, uint64_t>(
        LabelOrder::FORWARD,
        dims[0].get(),
        16,
        0,
        Datatype::UINT64,
        1,
        0,
        16,
        16,
        0);
    status = schema->add_dimension_label(0, "a", axis, true, false);
    INFO(status.to_string());
    REQUIRE(!status.ok());
  }

  SECTION("Catch shared label/attribute name with schema check") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0)};
    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto status = schema->check();
    auto axis = test::make_axis_schema<uint64_t, uint64_t>(
        LabelOrder::FORWARD,
        dims[0].get(),
        16,
        0,
        Datatype::UINT64,
        1,
        0,
        16,
        16,
        0);
    status = schema->add_dimension_label(0, "a", axis, false, true);
    REQUIRE(status.ok());
    status = schema->check();
    INFO(status.to_string());
    REQUIRE(!status.ok());
  }

  SECTION("Catch shared label/dimension name when adding label") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16),
        test::make_dimension<uint64_t>("y", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0)};
    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto status = schema->check();
    auto axis = test::make_axis_schema<uint64_t, uint64_t>(
        LabelOrder::FORWARD,
        dims[0].get(),
        16,
        0,
        Datatype::UINT64,
        1,
        0,
        16,
        16,
        0);
    status = schema->add_dimension_label(0, "y", axis, true, false);
    INFO(status.to_string());
    REQUIRE(!status.ok());
  }

  SECTION("Catch shared label/dimension name with check") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16),
        test::make_dimension<uint64_t>("y", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0)};
    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto status = schema->check();
    auto axis = test::make_axis_schema<uint64_t, uint64_t>(
        LabelOrder::FORWARD,
        dims[0].get(),
        16,
        0,
        Datatype::UINT64,
        1,
        0,
        16,
        16,
        0);
    status = schema->add_dimension_label(0, "y", axis, false, true);
    REQUIRE(status.ok());
    status = schema->check();
    INFO(status.to_string());
    REQUIRE(!status.ok());
  }

  SECTION("Catch shared label/dimension name when adding label") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16),
        test::make_dimension<uint64_t>("y", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0)};
    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto status = schema->check();
    auto axis = test::make_axis_schema<uint64_t, uint64_t>(
        LabelOrder::FORWARD,
        dims[0].get(),
        16,
        0,
        Datatype::UINT64,
        1,
        0,
        16,
        16,
        0);
    status = schema->add_dimension_label(0, "y", axis, true, false);
    INFO(status.to_string());
    REQUIRE(!status.ok());
  }

  SECTION("Catch shared label/dimension name with check") {
    std::vector<shared_ptr<Dimension>> dims{
        test::make_dimension<uint64_t>("x", Datatype::UINT64, 1, 0, 15, 16),
        test::make_dimension<uint64_t>("y", Datatype::UINT64, 1, 0, 15, 16)};
    std::vector<shared_ptr<Attribute>> attrs{
        test::make_attribute<float>("a", Datatype::UINT64, false, 1, 0)};
    auto schema = test::make_array_schema(ArrayType::DENSE, dims, attrs);
    auto status = schema->check();
    auto axis = test::make_axis_schema<uint64_t, uint64_t>(
        LabelOrder::FORWARD,
        dims[0].get(),
        16,
        0,
        Datatype::UINT64,
        1,
        0,
        16,
        16,
        0);
    status = schema->add_dimension_label(0, "y", axis, false, false);
    REQUIRE(status.ok());
    status = schema->check();
    INFO(status.to_string());
    REQUIRE(!status.ok());
  }
}
