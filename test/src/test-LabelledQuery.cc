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
#include "tiledb/sm/c_api/tiledb_struct_def.h"
#include "tiledb/sm/label_query/label_query.h"
#include "tiledb/sm/label_query/label_subarray.h"
#include "tiledb/sm/query/query.h"
#include "tiledb/sm/subarray/subarray.h"

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

/**
 * Create and write to a 1D array.
 *
 *  Domain: (dim0, [1, 16], uint64)
 *  Attrs: (a1, [0.1, 0.2, ...1.6], float)
 */
void create_main_array_1d(const std::string& name, tiledb_ctx_t* ctx) {
  uint64_t domain[2]{1, 16};
  uint64_t tile_extent{16};
  create_array(
      ctx,
      name,
      TILEDB_DENSE,
      {"dim0"},
      {TILEDB_UINT64},
      {domain},
      {&tile_extent},
      {"a1"},
      {TILEDB_FLOAT32},
      {1},
      {tiledb::test::Compressor(TILEDB_FILTER_LZ4, -1)},
      TILEDB_ROW_MAJOR,
      TILEDB_ROW_MAJOR,
      10000);
  tiledb::test::QueryBuffers buffers;
  std::vector<float> a1_data(16);
  for (uint64_t ii{0}; ii < 16; ++ii) {
    a1_data[ii] = 0.1 * (1 + ii);
  }
  buffers["a1"] = tiledb::test::QueryBuffer(
      {&a1_data[0], a1_data.size() * sizeof(float), nullptr, 0});
  write_array(ctx, name, TILEDB_ROW_MAJOR, buffers);
}

void create_uniform_label(const std::string& name, tiledb_ctx_t* ctx) {
  int64_t domain[2]{-16, -1};
  int64_t tile_extent{16};
  create_array(
      ctx,
      name,
      TILEDB_SPARSE,
      {"label"},
      {TILEDB_INT64},
      {domain},
      {&tile_extent},
      {"index"},
      {TILEDB_UINT64},
      {1},
      {tiledb::test::Compressor(TILEDB_FILTER_LZ4, -1)},
      TILEDB_ROW_MAJOR,
      TILEDB_ROW_MAJOR,
      10000);
  tiledb::test::QueryBuffers buffers;
  std::vector<int64_t> label_data(16);
  std::vector<uint64_t> index_data(16);
  for (uint32_t ii{0}; ii < 16; ++ii) {
    label_data[ii] = static_cast<int64_t>(ii) - 16;
    index_data[ii] = (ii + 1);
  }
  buffers["label"] = tiledb::test::QueryBuffer(
      {&label_data[0], label_data.size() * sizeof(int64_t), nullptr, 0});
  buffers["index"] = tiledb::test::QueryBuffer(
      {&index_data[0], index_data.size() * sizeof(uint64_t), nullptr, 0});
  write_array(ctx, name, TILEDB_GLOBAL_ORDER, buffers);
}

TEST_CASE_METHOD(
    TemporaryDirectoryFixture,
    "LabelledQuery: External label, 1D",
    "[Query][1d][DimensionLabel]") {
  // Create arrays for test cases.
  const std::string main_array_name = fullpath("main");
  const std::string label_array_name = fullpath("label");
  create_main_array_1d(main_array_name, ctx);
  create_uniform_label(label_array_name, ctx);
  SECTION("Standard query") {
    // Open the array
    tiledb_array_t* array;
    int rc = tiledb_array_alloc(ctx, main_array_name.c_str(), &array);
    CHECK(rc == TILEDB_OK);
    rc = tiledb_array_open(ctx, array, TILEDB_READ);
    CHECK(rc == TILEDB_OK);

    // Create and submit the query
    uint64_t subarray_vals[2]{4, 7};
    std::vector<float> a1(4);
    uint64_t a1_size{a1.size() * sizeof(float)};
    Query query{ctx->ctx_->storage_manager(), array->array_};
    query.set_layout(Layout::ROW_MAJOR);
    query.set_subarray(&subarray_vals[0]);
    query.set_data_buffer("a1", &a1[0], &a1_size);
    query.submit();

    CHECK(query.status() == QueryStatus::COMPLETED);

    // Close and clean-up the array
    rc = tiledb_array_close(ctx, array);
    CHECK(rc == TILEDB_OK);
    tiledb_array_free(&array);

    // Check results.
    for (uint64_t ii{0}; ii < 4; ++ii) {
      CHECK(a1[ii] == static_cast<float>(0.1 * (subarray_vals[0] + ii)));
    }
  }
  SECTION("Direct query on label") {
    // Open the label array
    tiledb_array_t* label_array;
    auto rc = tiledb_array_alloc(ctx, label_array_name.c_str(), &label_array);
    CHECK(rc == TILEDB_OK);
    rc = tiledb_array_open(ctx, label_array, TILEDB_READ);
    CHECK(rc == TILEDB_OK);

    // Create subarray.
    Subarray subarray{label_array->array_,
                      Layout::ROW_MAJOR,
                      nullptr,
                      ctx->ctx_->storage_manager()->logger(),
                      true,
                      ctx->ctx_->storage_manager()};
    std::vector<int64_t> range{-8, -5};
    subarray.add_range(0, &range[0], &range[1], nullptr);

    // Create query.
    Query query{ctx->ctx_->storage_manager(), label_array->array_};
    query.set_subarray(subarray);
    std::vector<int64_t> label(4);
    uint64_t label_size{label.size() * sizeof(int64_t)};
    query.set_data_buffer("label", &label[0], &label_size);
    std::vector<uint64_t> index(4);
    uint64_t index_size{index.size() * sizeof(uint64_t)};
    query.set_data_buffer("index", &index[0], &index_size);
    query.submit();

    CHECK(query.status() == QueryStatus::COMPLETED);

    // Close and clean-up the array
    rc = tiledb_array_close(ctx, label_array);
    CHECK(rc == TILEDB_OK);
    tiledb_array_free(&label_array);

    // Check results
    std::vector<int64_t> expected_label{-8, -7, -6, -5};
    std::vector<uint64_t> expected_index{9, 10, 11, 12};

    for (size_t ii{0}; ii < 4; ++ii) {
      INFO("Label " << std::to_string(ii));
      CHECK(label[ii] == expected_label[ii]);
      CHECK(index[ii] == expected_index[ii]);
    }
  }
  SECTION("Labelled query") {
    // Open the array
    tiledb_array_t* main_array;
    int rc = tiledb_array_alloc(ctx, main_array_name.c_str(), &main_array);
    CHECK(rc == TILEDB_OK);
    rc = tiledb_array_open(ctx, main_array, TILEDB_READ);
    CHECK(rc == TILEDB_OK);

    // Open the label array
    tiledb_array_t* label_array;
    rc = tiledb_array_alloc(ctx, label_array_name.c_str(), &label_array);
    CHECK(rc == TILEDB_OK);
    rc = tiledb_array_open(ctx, label_array, TILEDB_READ);
    CHECK(rc == TILEDB_OK);

    // Create subarray.
    LabelledSubarray subarray{main_array->array_,
                              nullptr,
                              ctx->ctx_->storage_manager()->logger(),
                              true,
                              ctx->ctx_->storage_manager()};
    subarray.set_external_label(
        0,
        "label0",
        LabelOrderType::UNORDERED,
        "label",
        "index",
        label_array->array_);
    std::vector<int64_t> range{-8, -5};
    subarray.add_label_range(0, &range[0], &range[1], nullptr);

    // Create query.
    LabelledQuery query{
        subarray, ctx->ctx_->storage_manager(), main_array->array_};
    std::vector<int64_t> label(4);
    uint64_t label_size{label.size() * sizeof(int64_t)};
    std::vector<uint64_t> index(4);
    uint64_t index_size{index.size() * sizeof(uint64_t)};
    query.set_data_buffer("dim0", &index[0], &index_size);
    query.set_label_data_buffer("label0", &label[0], &label_size);

    // Submit label query and check for success.
    // query.submit_labels();
    //

    // Submit main query and check for success.

    // Close and clean-up the arrayis
    rc = tiledb_array_close(ctx, main_array);
    CHECK(rc == TILEDB_OK);
    tiledb_array_free(&main_array);
    // Close and clean-up the array
    rc = tiledb_array_close(ctx, label_array);
    CHECK(rc == TILEDB_OK);
    tiledb_array_free(&label_array);
  }
}
