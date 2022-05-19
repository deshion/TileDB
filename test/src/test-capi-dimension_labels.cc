/**
 * @file test-LabelledQuery.cc
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
 * Tests the `LabelledQuery` class.
 */

#include "test/src/helpers.h"
#include "test/src/vfs_helpers.h"
#include "tiledb/sm/c_api/tiledb.h"
#include "tiledb/sm/c_api/tiledb_dimension_label_experimental.h"
#include "tiledb/sm/c_api/tiledb_struct_def.h"

#ifdef _WIN32
#include "tiledb/sm/filesystem/win.h"
#else
#include "tiledb/sm/filesystem/posix.h"
#endif

#include <catch.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace tiledb::sm;
using namespace tiledb::test;

/* ********************************* */
/*         STRUCT DEFINITION         */
/* ********************************* */

/**
 * Fixture for creating a temporary directory for a test case. This fixture
 * also manages the context and virtual file system for the test case.
 */
struct TemporaryDirectoryFixture {
 public:
  /** Fixture constructor. */
  TemporaryDirectoryFixture()
      : supported_filesystems_(vfs_test_get_fs_vec()) {
    // Initialize virtual filesystem and context.
    REQUIRE(vfs_test_init(supported_filesystems_, &ctx, &vfs_).ok());

    // Create temporary directory based on the supported filesystem
#ifdef _WIN32
    SupportedFsLocal windows_fs;
    temp_dir_ = windows_fs.file_prefix() + windows_fs.temp_dir();
#else
    SupportedFsLocal posix_fs;
    temp_dir_ = posix_fs.file_prefix() + posix_fs.temp_dir();
#endif
    create_dir(temp_dir_, ctx, vfs_);
  }

  /** Fixture destructor. */
  ~TemporaryDirectoryFixture() {
    remove_dir(temp_dir_, ctx, vfs_);
    tiledb_ctx_free(&ctx);
    tiledb_vfs_free(&vfs_);
  }

  /** Create a new array. */
  std::string fullpath(std::string&& name) {
    return temp_dir_ + name;
  }

 protected:
  /** TileDB context */
  tiledb_ctx_t* ctx;

  /** Name of the temporary directory to use for this test */
  std::string temp_dir_;

 private:
  /** Virtual file system */
  tiledb_vfs_t* vfs_;

  /** Vector of supported filesystems. Used to initialize ``vfs_``. */
  const std::vector<std::unique_ptr<SupportedFs>> supported_filesystems_;
};

TEST_CASE_METHOD(
    TemporaryDirectoryFixture,
    "Load array schema with labels",
    "[ArraySchema][DimensionLabel]") {
  // Create array schema
  uint64_t x_domain[2]{0, 63};
  uint64_t x_tile_extent{64};
  uint64_t y_domain[2]{0, 63};
  uint64_t y_tile_extent{64};
  auto array_schema = create_array_schema(
      ctx,
      TILEDB_DENSE,
      {"x", "y"},
      {TILEDB_UINT64, TILEDB_UINT64},
      {&x_domain[0], &y_domain[0]},
      {&x_tile_extent, &y_tile_extent},
      {"a"},
      {TILEDB_FLOAT64},
      {1},
      {tiledb::test::Compressor(TILEDB_FILTER_NONE, -1)},
      TILEDB_ROW_MAJOR,
      TILEDB_ROW_MAJOR,
      4096,
      false);
  double label1_domain[2]{-10.0, 10.0};
  double label1_tile_extent{5.0};
  auto index1_array_schema = create_array_schema(
      ctx,
      TILEDB_DENSE,
      {"index"},
      {TILEDB_UINT64},
      {&x_domain[0]},
      {&x_tile_extent},
      {"label"},
      {TILEDB_FLOAT64},
      {1},
      {tiledb::test::Compressor(TILEDB_FILTER_NONE, -1)},
      TILEDB_ROW_MAJOR,
      TILEDB_ROW_MAJOR,
      4096,
      false);
  auto label1_array_schema = create_array_schema(
      ctx,
      TILEDB_SPARSE,
      {"label"},
      {TILEDB_FLOAT64},
      {&label1_domain[0]},
      {&label1_tile_extent},
      {"index"},
      {TILEDB_UINT64},
      {1},
      {tiledb::test::Compressor(TILEDB_FILTER_NONE, -1)},
      TILEDB_ROW_MAJOR,
      TILEDB_ROW_MAJOR,
      4096,
      false);
  REQUIRE_TILEDB_OK(tiledb_array_schema_add_dimension_label(
      ctx,
      array_schema,
      0,
      "x",
      TILEDB_FORWARD,
      index1_array_schema,
      label1_array_schema));

  // Check array schema
  REQUIRE_TILEDB_OK(tiledb_array_schema_check(ctx, array_schema));
  uint32_t dim_label_num;
  REQUIRE_TILEDB_OK(
      tiledb_array_schema_get_dim_label_num(ctx, array_schema, &dim_label_num));
  REQUIRE(dim_label_num == 1);

  // Create array with
  std::string array_name{fullpath("array")};
  REQUIRE_TILEDB_OK(tiledb_array_create(ctx, array_name.c_str(), array_schema));

  // Load array schema and check labels.
  tiledb_array_schema_t* loaded_array_schema{nullptr};
  REQUIRE_TILEDB_OK(
      tiledb_array_schema_load(ctx, array_name.c_str(), &loaded_array_schema));
  REQUIRE_TILEDB_OK(tiledb_array_schema_check(ctx, loaded_array_schema));

  // Clean up
  tiledb_array_schema_free(&array_schema);
  tiledb_array_schema_free(&index1_array_schema);
  tiledb_array_schema_free(&label1_array_schema);
  tiledb_array_schema_free(&loaded_array_schema);
}
