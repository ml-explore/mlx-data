/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "bxzstr.hpp"

#if defined(BXZSTR_Z_SUPPORT) && (BXZSTR_Z_SUPPORT) == 1

#ifndef BXZSTR_Z_STREAM_WRAPPER_UNITTEST_HPP
#define BXZSTR_Z_STREAM_WRAPPER_UNITTEST_HPP

#include <vector>
#include <string>
#include <cstddef>

#include "gtest/gtest.h"
#include "zlib.h"

// Test zException
class ZExceptionTest : public ::testing::Test {
  protected:
    void SetUp() override {
	this->msgConstructorValue = "urpdcjgztzcowdpiucfrhxczlgbbopeg";
	this->msgConstructorExpected = "urpdcjgztzcowdpiucfrhxczlgbbopeg";
	this->errcodeConstructorValue = -1;
	this->errcodeInputs = std::vector<int>({ Z_STREAM_ERROR, Z_DATA_ERROR, Z_MEM_ERROR, Z_VERSION_ERROR, Z_BUF_ERROR, -1 });
	this->errcodeExpecteds = std::vector<std::string>({
		"zlib: Z_STREAM_ERROR: urpdcjgztzcowdpiucfrhxczlgbbopeg",
		"zlib: Z_DATA_ERROR: urpdcjgztzcowdpiucfrhxczlgbbopeg",
		"zlib: Z_MEM_ERROR: urpdcjgztzcowdpiucfrhxczlgbbopeg",
		"zlib: Z_VERSION_ERROR: urpdcjgztzcowdpiucfrhxczlgbbopeg",
		"zlib: Z_BUF_ERROR: urpdcjgztzcowdpiucfrhxczlgbbopeg",
		"zlib: [-1]: urpdcjgztzcowdpiucfrhxczlgbbopeg"});
    }
    void TearDown() override {
    }

    // Test inputs
    std::vector<int> errcodeInputs;
    // Test values
    std::string msgConstructorValue;
    size_t errcodeConstructorValue;

    // Expecteds
    std::string msgConstructorExpected;
    std::vector<std::string> errcodeExpecteds;

};

// Test z_stream_wrapper
class ZStreamWrapperTest : public ::testing::Test {
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
class ZCompressAndDecompressTest {
  protected:
    unsigned char test_vals[34] = {0x1f, 0x8b, 0x08, 0x08, 0xf1, 0x0a, 0x61, 0x62, 0x00, 0x03, 0x74, 0x65, 0x73, 0x74, 0x7a, 0x2e,
	                         0x74, 0x78, 0x74, 0x00, 0x33, 0xe4, 0x32, 0xc4, 0x80, 0x00, 0x4c, 0xd2, 0xca, 0x03, 0x14, 0x00,
				 0x00, 0x00 };
    unsigned char output_vals[10] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };

    unsigned char* testIn;
    const unsigned char* testOut;
    bxz::detail::z_stream_wrapper* wrapper;

    void set_addresses(bxz::detail::z_stream_wrapper* wrapper) {
	wrapper->set_next_in(&testIn[0]);
	wrapper->set_avail_in(10);
	wrapper->set_next_out(&testOut[0]);
	wrapper->set_avail_out(10);
    }
};

// Test decompress
class ZDecompressTest : public ZCompressAndDecompressTest, public ::testing::Test {
  protected:
    void SetUp() override {
	this->testIn = reinterpret_cast<unsigned char*>(test_vals);
	this->testOut = reinterpret_cast<const unsigned char*>(output_vals);
	wrapper = new bxz::detail::z_stream_wrapper();
	this->set_addresses(wrapper);
    }
    void TearDown() override {
    }
};

// Test compress
class ZCompressTest : public ZCompressAndDecompressTest, public ::testing::Test {
  protected:
    void SetUp() override {
	this->testIn = reinterpret_cast<unsigned char*>(test_vals);
	this->testOut = reinterpret_cast<const unsigned char*>(output_vals);
	wrapper = new bxz::detail::z_stream_wrapper(false);
	this->set_addresses(wrapper);
    }
    void TearDown() override {
    }
};

#endif
#endif
