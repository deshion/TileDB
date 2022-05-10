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
  uint64_t tile_extent{4};
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
    a1_data[ii] = 0.1 * ii;
  }
  buffers["a1"] = tiledb::test::QueryBuffer(
      {&a1_data[0], a1_data.size() * sizeof(float), nullptr, 0});
  write_array(ctx, name, TILEDB_ROW_MAJOR, buffers);
}

void create_uniform_label(const std::string& name, tiledb_ctx_t* ctx) {
  double domain[2]{-100.0, 100.0};
  double tile_extent{10.0};
  create_array(
      ctx,
      name,
      TILEDB_SPARSE,
      {"label"},
      {TILEDB_FLOAT64},
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
  std::vector<double> label_data(16);
  std::vector<uint64_t> index_data(16);
  for (uint64_t ii{0}; ii < 16; ++ii) {
    label_data[ii] = ii * 0.5;
    index_data[ii] = ii;
  }
  buffers["label"] = tiledb::test::QueryBuffer(
      {&label_data[0], label_data.size() * sizeof(double), nullptr, 0});
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
    //   // Set array configuration
    //   tiledb_array_t* array;
    //   int rc = tiledb_array_alloc(ctx, array_name.c_str(), &array);
    //   CHECK(rc == TILEDB_OK);
    //   tiledb_config_t* cfg;
    //   tiledb_error_t* err = nullptr;
    //   REQUIRE(tiledb_config_alloc(&cfg, &err) == TILEDB_OK);
    //   REQUIRE(err == nullptr);

    //   rc = tiledb_array_open(ctx, array, TILEDB_READ);
    //   CHECK(rc == TILEDB_OK);

    //   // Create query
    //   tiledb_query_t* query;
    //   rc = tiledb_query_alloc(ctx, array, TILEDB_READ, &query);
    //   CHECK(rc == TILEDB_OK);
    //   rc = tiledb_query_set_layout(ctx, query, layout);
    //   CHECK(rc == TILEDB_OK);

    //   // Set buffers

    //   for (const auto& b : buffers) {
    //     if (b.second.var_ == nullptr) {  // Fixed-sized
    //       rc = tiledb_query_set_data_buffer(
    //           ctx,
    //           query,
    //           b.first.c_str(),
    //           b.second.fixed_,
    //           (uint64_t*)&(b.second.fixed_size_));
    //       CHECK(rc == TILEDB_OK);
    //     } else {  // Var-sized
    //       rc = tiledb_query_set_data_buffer(
    //           ctx,
    //           query,
    //           b.first.c_str(),
    //           b.second.var_,
    //           (uint64_t*)&(b.second.var_size_));
    //       CHECK(rc == TILEDB_OK);
    //       rc = tiledb_query_set_offsets_buffer(
    //           ctx,
    //           query,
    //           b.first.c_str(),
    //           (uint64_t*)b.second.fixed_,
    //           (uint64_t*)&(b.second.fixed_size_));
    //       CHECK(rc == TILEDB_OK);
    //     }
    //   }

    //   // Submit query
    //   rc = tiledb_query_submit(ctx, query);
    //   CHECK(rc == TILEDB_OK);

    //   // Finalize query
    //   rc = tiledb_query_finalize(ctx, query);
    //   CHECK(rc == TILEDB_OK);

    //   // Get fragment uri
    //   const char* temp_uri;
    //   rc = tiledb_query_get_fragment_uri(ctx, query, 0, &temp_uri);
    //   REQUIRE(rc == TILEDB_OK);
    //   *uri = std::string(temp_uri);

    //   // Close array
    //   rc = tiledb_array_close(ctx, array);
    //   CHECK(rc == TILEDB_OK);

    //   // Clean up
    //   tiledb_array_free(&array);
    //   tiledb_query_free(&query);
    //   tiledb_config_free(&cfg);
  }
  SECTION("Labelled query") {
  }
}
