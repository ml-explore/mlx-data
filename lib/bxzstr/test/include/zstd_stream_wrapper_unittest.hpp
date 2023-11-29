/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "bxzstr.hpp"

#if defined(BXZSTR_ZSTD_SUPPORT) && (BXZSTR_ZSTD_SUPPORT) == 1

#ifndef BXZSTR_ZSTD_STREAM_WRAPPER_UNITTEST_HPP
#define BXZSTR_ZSTD_STREAM_WRAPPER_UNITTEST_HPP

#include <string>
#include <cstddef>

#include "gtest/gtest.h"
#include "zstd.h"


// Test zstdException
class ZstdExceptionTest : public ::testing::Test {
  protected:
    void SetUp() override {
	this->msgConstructorValue = "urpdcjgztzcowdpiucfrhxczlgbbopeg";
	this->msgConstructorExpected = "urpdcjgztzcowdpiucfrhxczlgbbopeg";
	this->errcodeConstructorValue = -1;
	this->errcodeConstructorExpected = "zstd error: [18446744073709551615]: Error (generic)";
    }
    void TearDown() override {
    }
    // Test values
    std::string msgConstructorValue;
    size_t errcodeConstructorValue;
    // Expecteds
    std::string msgConstructorExpected;
    std::string errcodeConstructorExpected;
};

// Test zstd_stream_wrapper
class ZstdStreamWrapperTest : public ::testing::Test {
  protected:
    void SetUp() override {
	this->testTrue = true;
	this->testFalse = false;
    }
    void TearDown() override {
    }
    // Test values
    bool testTrue;
    bool testFalse;
};

// Common inputs/outputs for compression and decompression testing
class ZstdCompressAndDecompressTest {
  protected:
    unsigned char test_vals[26] = { 0x28, 0xb5, 0x2f, 0xfd, 0x00, 0x58, 0x45, 0x00, 0x00, 0x10, 0x31, 0x0a, 0x01, 0x00, 0x79, 0x0e,
	                            0x0b, 0x28, 0xb5, 0x2f, 0xfd, 0x20, 0x00, 0x01, 0x00, 0x00 };
    unsigned char output_vals[10] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };

    unsigned char* testIn;
    const unsigned char* testOut;
    bxz::detail::zstd_stream_wrapper* wrapper;

    void set_addresses(bxz::detail::zstd_stream_wrapper* wrapper) {
	wrapper->set_next_in(&testIn[0]);
	wrapper->set_avail_in(10);
	wrapper->set_next_out(&testOut[0]);
	wrapper->set_avail_out(10);
    }
};

// Test decompress
class ZstdDecompressTest : public ZstdCompressAndDecompressTest, public ::testing::Test {
  protected:
    void SetUp() override {
	this->testIn = reinterpret_cast<unsigned char*>(test_vals);
	this->testOut = reinterpret_cast<const unsigned char*>(output_vals);
	wrapper = new bxz::detail::zstd_stream_wrapper();
	this->set_addresses(wrapper);
    }
    void TearDown() override {
    }
};

// Test compress
class ZstdCompressTest : public ZstdCompressAndDecompressTest, public ::testing::Test {
  protected:
    void SetUp() override {
	this->testIn = reinterpret_cast<unsigned char*>(test_vals);
	this->testOut = reinterpret_cast<const unsigned char*>(output_vals);
	wrapper = new bxz::detail::zstd_stream_wrapper(false);
	this->set_addresses(wrapper);
    }
    void TearDown() override {
    }
};

#endif

#endif
