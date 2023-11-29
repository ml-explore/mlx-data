/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "bxzstr.hpp"

#if defined(BXZSTR_BZ2_SUPPORT) && (BXZSTR_BZ2_SUPPORT) == 1

#ifndef BXZSTR_LZMA_STREAM_WRAPPER_UNITTEST_HPP
#define BXZSTR_LZMA_STREAM_WRAPPER_UNITTEST_HPP

#include <vector>
#include <string>
#include <cstddef>

#include "gtest/gtest.h"
#include "lzma.h"

// Test zException
class LzmaExceptionTest : public ::testing::Test {
  protected:
    void SetUp() override {
	this->msgConstructorValue = "urpdcjgztzcowdpiucfrhxczlgbbopeg";
	this->msgConstructorExpected = "urpdcjgztzcowdpiucfrhxczlgbbopeg";
	this->errcodeConstructorValue = -1;
	this->errcodeInputs = std::vector<lzma_ret>({ LZMA_MEM_ERROR, LZMA_OPTIONS_ERROR, LZMA_UNSUPPORTED_CHECK, LZMA_PROG_ERROR, LZMA_BUF_ERROR, LZMA_DATA_ERROR, LZMA_FORMAT_ERROR, LZMA_NO_CHECK, LZMA_MEMLIMIT_ERROR });
	this->errcodeExpecteds = std::vector<std::string>({
		"liblzma: LZMA_MEM_ERROR: \x5",
		"liblzma: LZMA_OPTIONS_ERROR: \b",
		"liblzma: LZMA_UNSUPPORTED_CHECK: \x3",
		"liblzma: LZMA_PROG_ERROR: \v",
		"liblzma: LZMA_BUF_ERROR: \n",
		"liblzma: LZMA_DATA_ERROR: \t",
		"liblzma: LZMA_FORMAT_ERROR: \a",
		"liblzma: LZMA_NO_CHECK: \x2",
		"liblzma: LZMA_MEMLIMIT_ERROR: \x6"
	    });
    }
    void TearDown() override {
    }

    // Test inputs
    std::vector<lzma_ret> errcodeInputs;
    // Test values
    std::string msgConstructorValue;
    size_t errcodeConstructorValue;

    // Expecteds
    std::string msgConstructorExpected;
    std::vector<std::string> errcodeExpecteds;

};

// Test z_stream_wrapper
class LzmaStreamWrapperTest : public ::testing::Test {
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
class LzmaCompressAndDecompressTest {
  protected:
    unsigned char test_vals[68] = { 0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00, 0x00, 0x04, 0xe6, 0xd6, 0xb4, 0x46, 0x02, 0x00, 0x21, 0x01,
	                            0x16, 0x00, 0x00, 0x00, 0x74, 0x2f, 0xe5, 0xa3, 0xe0, 0x00, 0x13, 0x00, 0x08, 0x5d, 0x00, 0x18,
				    0x82, 0xa7, 0x83, 0x80, 0x00, 0x00, 0x00, 0x00, 0xc3, 0x65, 0xdc, 0x8f, 0x27, 0x35, 0xda, 0x98,
				    0x00, 0x01, 0x24, 0x14, 0xf0, 0x80, 0xb4, 0xcb, 0x1f, 0xb6, 0xf3, 0x7d, 0x01, 0x00, 0x00, 0x00,
				    0x00, 0x04, 0x59, 0x5a };
    unsigned char output_vals[10] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };

    unsigned char* testIn;
    const unsigned char* testOut;
    bxz::detail::lzma_stream_wrapper* wrapper;

    void set_addresses(bxz::detail::lzma_stream_wrapper* wrapper) {
	wrapper->set_next_in(&testIn[0]);
	wrapper->set_avail_in(10);
	wrapper->set_next_out(&testOut[0]);
	wrapper->set_avail_out(10);
    }
};

// Test decompress
class LzmaDecompressTest : public LzmaCompressAndDecompressTest, public ::testing::Test {
  protected:
    void SetUp() override {
	this->testIn = reinterpret_cast<unsigned char*>(test_vals);
	this->testOut = reinterpret_cast<const unsigned char*>(output_vals);
	wrapper = new bxz::detail::lzma_stream_wrapper();
	this->set_addresses(wrapper);
    }
    void TearDown() override {
    }
};

// Test compress
class LzmaCompressTest : public LzmaCompressAndDecompressTest, public ::testing::Test {
  protected:
    void SetUp() override {
	this->testIn = reinterpret_cast<unsigned char*>(test_vals);
	this->testOut = reinterpret_cast<const unsigned char*>(output_vals);
	wrapper = new bxz::detail::lzma_stream_wrapper(false);
	this->set_addresses(wrapper);
    }
    void TearDown() override {
    }
};

#endif

#endif
