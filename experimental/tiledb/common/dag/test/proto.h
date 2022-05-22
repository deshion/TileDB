/**
 * @file experimental/tiledb/common/dag/test/proto.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2021 TileDB, Inc.
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
 * This file defines some elementary node types for testing
 */

#ifndef TILEDB_PROTO_H
#define TILEDB_PROTO_H

#include <atomic>
#include "../dag.h"

namespace tiledb::common {

template <class Block = size_t>
class generator {
  static std::atomic<Block> i_{0};

 public:
  Block operator()() {
    return i_++;
  }
};

/**
 * Prototype source node.  Constructed with a function that creates items.
 */
template <class Block = size_t>
class producer_node : public Source<Block> {
  using Base = Source<Block>;
  std::function<Block()> f_;

 public:
  /**
   * Constructor
   * @param f A function that accepts items.
   * @tparam The type of the function (or function object) that generates items.
   */
  template <class Function>
  producer_node(Function&& f)
      : f_{std::forward<Function>(f)} {
  }

  /**
   * Generate an output.
   */
  void run() {
    Base::item_ = f_();
  }
};

/**
 * Consumer function object class.  Takes items and puts them on an Output
 * Iterator.
 */
template <class OutputIterator, class Block = size_t>
class consumer {
  OutputIterator iter_;

 public:
  consumer(OutputIterator iter)
      : iter_(iter) {
  }
  void operator()(Block& item) {
    *iter_++ = item;
  }
};

/**
 * A proto consumer node.  Constructed with a function that accepts items.
 */
template <class Block = size_t>
class consumer_node : public Sink<Block> {
  using Base = Sink<Block>;
  std::function<void(Block)> f_;

 public:
  /**
   * Constructor
   * @param f A function that accepts items.
   * @tparam The type of the function (or function object) that accepts items.
   */
  template <class Function>
  consumer_node(Function&& f)
      : f_{std::forward<Function>(f)} {
  }

  /**
   *  Receive an item from a Source
   */
  void run() {
    f_(Base::item_);
  }
};
}  // namespace tiledb::common
#endif  // TILEDB_PROTO_H
