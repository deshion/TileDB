/**
 * @file tiledb/sm/c_api/tiledb_dimension_label_experimental.h
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
 * Experimental C-API for dimension labels.
 */

#ifndef TILEDB_DIMENSION_LABEL_EXPERIMENTAL_H
#define TILEDB_DIMENSION_LABEL_EXPERIMENTAL_H

#include "tiledb.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
#define TILEDB_LABEL_ORDER_ENUM(id) TILEDB_##id
#include "tiledb_enum_experimental.h"
#undef TILEDB_LABEL_ORDER_ENUM
} tiledb_label_order_t;

// TODO: Decide how to add dimension label to dimension label.
//
// Options:
// * Specify 2 ArraySchema objects
// * Specify 2 Dimensions and 2 Attributes objevts
// * Specify 2 AxisComponent objects
//   - Requires adding tiledb_axis_component_t type
// * Specify AxisSchema object
//   - Requires adding tiledb_axis_schema_t type
/**
 * Adds an axis to a array schema as a dimension label.
 *
 * @code{.c}
 * tiledb_array_schema_t* index_array_schema;
 * tiledb_array_schema_t* label_array_schema;
 * // TODO finish example code
 * // ...
 * tiledb_array_schema_add_dimension_label(
 *     ctx,
 *     array_schema,
 *     0,
 *     "label1",
 *     TILEDB_LABEL_ORDER_FORWARD,
 *     index_array_schema,
 *     label_array_schema);
 * @endcode
 *
 */
TILEDB_EXPORT int32_t tiledb_array_schema_add_dimension_label(
    tiledb_ctx_t* ctx,
    tiledb_array_schema_t* array_schema,
    uint32_t dim_id,
    const char* name,
    tiledb_label_order_t label_order,
    tiledb_array_schema_t* index_array_schema,
    tiledb_array_schema_t* label_array_schema);

/**
 *
 * Retrieves the number of array dimension labels.
 *
 * **Example:**
 *
 * @code{.c}
 * uint32_t dim_label_num;
 * tiledb_array_schema_get_dim_label_num(ctx, array_schema, &dim_label_num);
 * @endcode
 *
 * @param ctx The TileDB context.
 * @param array_schema The array schema.
 * @param attribute_num The number of attributes to be retrieved.
 * @return `TILEDB_OK` for success and `TILEDB_ERR` for error.
 */
TILEDB_EXPORT int32_t tiledb_array_schema_get_dim_label_num(
    tiledb_ctx_t* ctx,
    const tiledb_array_schema_t* array_schema,
    uint32_t* dim_label_num);

/**
 * Checks whether the array schema has a dimension label of the given name.
 *
 * **Example:**
 *
 * @code{.c}
 * int32_t has_dim_label;
 * tiledb_array_schema_has_dim_label(ctx, array_schema, "label_0",
 * &has_dim_label);
 * @endcode
 *
 * @param ctx The TileDB context.
 * @param array_schema The array schema.
 * @param name The name of the attribute to check for.
 * @param has_attr Set to `1` if the array schema has an attribute of the
 *      given name, else `0`.
 * @return `TILEDB_OK` for success and `TILEDB_ERR` for error.
 */
TILEDB_EXPORT int32_t tiledb_array_schema_has_dim_label(
    tiledb_ctx_t* ctx,
    const tiledb_array_schema_t* array_schema,
    const char* name,
    int32_t* has_dim_label);
}

#endif
