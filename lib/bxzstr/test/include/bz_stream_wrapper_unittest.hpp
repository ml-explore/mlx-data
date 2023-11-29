/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "bxzstr.hpp"

#if defined(BXZSTR_BZ2_SUPPORT) && (BXZSTR_BZ2_SUPPORT) == 1

#ifndef BXZSTR_BZ_STREAM_WRAPPER_UNITTEST_HPP
#define BXZSTR_BZ_STREAM_WRAPPER_UNITTEST_HPP

#include <vector>
#include <string>
#include <cstddef>

#include "gtest/gtest.h"
#include "bzlib.h"

// Test zException
class BzExceptionTest : public ::testing::Test {
  protected:
    void SetUp() override {
	this->msgConstructorValue = "urpdcjgztzcowdpiucfrhxczlgbbopeg";
	this->msgConstructorExpected = "urpdcjgztzcowdpiucfrhxczlgbbopeg";
	this->errcodeConstructorValue = -1;
	this->errcodeInputs = std::vector<int>({ BZ_CONFIG_ERROR, BZ_SEQUENCE_ERROR, BZ_PARAM_ERROR, BZ_MEM_ERROR, BZ_DATA_ERROR, BZ_DATA_ERROR_MAGIC, BZ_IO_ERROR, BZ_UNEXPECTED_EOF, BZ_OUTBUFF_FULL, -1000 });
	this->errcodeExpecteds = std::vector<std::string>({
		"bzlib: BZ_CONFIG_ERROR: \xF7",
		"bzlib: BZ_SEQUENCE_ERROR: \xFF",
		"bzlib: BZ_PARAM_ERROR: \xFE",
		"bzlib: BZ_MEM_ERROR: \xFD",
		"bzlib: BZ_DATA_ERROR: \xFC",
		"bzlib: BZ_DATA_ERROR_MAGIC: \xFB",
		"bzlib: BZ_IO_ERROR: \xFA",
		"bzlib: BZ_UNEXPECTED_EOF: \xF9",
		"bzlib: BZ_OUTBUFF_FULL: \xF8",
		"bzlib: [-1000]: \x18"
	    });
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
class BzStreamWrapperTest : public ::testing::Test {
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
class BzCompressAndDecompressTest {
  protected:
    unsigned char test_vals[41] = { 0x42, 0x5a, 0x68, 0x39, 0x31, 0x41, 0x59, 0x26, 0x53, 0x59, 0x35, 0xaa, 0x83, 0x68, 0x00, 0x00,
	                            0x09, 0xc8, 0x00, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x20, 0xa9, 0xa0, 0x82, 0x64, 0xce, 0x2e,
				    0xe4, 0x8a, 0x70, 0xa1, 0x20, 0x6b, 0x55, 0x06, 0xd0 };
    unsigned char output_vals[10] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };

    unsigned char* testIn;
    const unsigned char* testOut;
    bxz::detail::bz_stream_wrapper* wrapper;

    void set_addresses(bxz::detail::bz_stream_wrapper* wrapper) {
	wrapper->set_next_in(&testIn[0]);
	wrapper->set_avail_in(10);
	wrapper->set_next_out(&testOut[0]);
	wrapper->set_avail_out(10);
    }
};

// Test decompress
class BzDecompressTest : public BzCompressAndDecompressTest, public ::testing::Test {
  protected:
    void SetUp() override {
	this->testIn = reinterpret_cast<unsigned char*>(test_vals);
	this->testOut = reinterpret_cast<const unsigned char*>(output_vals);
	wrapper = new bxz::detail::bz_stream_wrapper();
	this->set_addresses(wrapper);
    }
    void TearDown() override {
    }
};

// Test compress
class BzCompressTest : public BzCompressAndDecompressTest, public ::testing::Test {
  protected:
    void SetUp() override {
	this->testIn = reinterpret_cast<unsigned char*>(test_vals);
	this->testOut = reinterpret_cast<const unsigned char*>(output_vals);
	wrapper = new bxz::detail::bz_stream_wrapper(false);
	this->set_addresses(wrapper);
    }
    void TearDown() override {
    }
};

#endif

#endif
