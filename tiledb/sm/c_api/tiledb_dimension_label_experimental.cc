/**
 * @file tiledb/sm/c_api/tiledb_dimension_label_experimental.cc
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

#include "tiledb/sm/c_api/tiledb_dimension_label_experimental.h"
#include "api_exception_safety.h"
#include "tiledb/sm/c_api/tiledb.h"

using namespace tiledb::common;

int32_t tiledb_array_schema_add_dimension_label(
    tiledb_ctx_t* ctx,
    tiledb_array_schema_t* array_schema,
    uint32_t dim_id,
    const char* name,
    tiledb_label_order_t label_order,
    tiledb_array_schema_t* index_array_schema,
    tiledb_array_schema_t* label_array_schema) {
  if (sanity_check(ctx) == TILEDB_ERR || sanity_check(ctx, array_schema) ||
      sanity_check(ctx, index_array_schema) ||
      sanity_check(ctx, label_array_schema))
    return TILEDB_ERR;
  /** Note: The call to make_shared creates a copy of the array schemas and
   * the user-visible handles no longer refere to the same objects in the
   *array schema.
   **/
  if (SAVE_ERROR_CATCH(
          ctx,
          array_schema->array_schema_->add_dimension_label(
              dim_id,
              name,
              static_cast<tiledb::sm::LabelOrder>(label_order),
              make_shared<tiledb::sm::ArraySchema>(
                  HERE(), *index_array_schema->array_schema_),
              make_shared<tiledb::sm::ArraySchema>(
                  HERE(), *label_array_schema->array_schema_))))
    return TILEDB_ERR;
  return TILEDB_OK;
}

int32_t tiledb_array_schema_get_dim_label_num(
    tiledb_ctx_t* ctx,
    const tiledb_array_schema_t* array_schema,
    uint32_t* dim_label_num) {
  if (sanity_check(ctx) == TILEDB_ERR ||
      sanity_check(ctx, array_schema) == TILEDB_ERR)
    return TILEDB_ERR;
  *dim_label_num = array_schema->array_schema_->dim_label_num();
  return TILEDB_OK;
}

int32_t tiledb_array_schema_has_dim_label(
    tiledb_ctx_t* ctx,
    const tiledb_array_schema_t* array_schema,
    const char* name,
    int32_t* has_dim_label) {
  if (sanity_check(ctx) == TILEDB_ERR ||
      sanity_check(ctx, array_schema) == TILEDB_ERR) {
    return TILEDB_ERR;
  }
  bool is_dim_label = array_schema->array_schema_->is_dim_label(name);
  *has_dim_label = is_dim_label ? 1 : 0;
  return TILEDB_OK;
}
