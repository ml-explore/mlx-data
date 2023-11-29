/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#ifndef BXZSTR_BXZSTR_IFSTREAM_INTEGRATIONTEST_HPP
#define BXZSTR_BXZSTR_IFSTREAM_INTEGRATIONTEST_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

#include "gtest/gtest.h"

#include "bxzstr.hpp"

// Integration tests for bxz::ifstream
//
// Define some vals that are used in all decompression tests
class DecompressionTest {
  protected:
    // Expected values
    static uint32_t n_in_vals;
    static std::vector<char> expected;

    // Test parameters
    std::string test_infile;

    void write_test_data(const unsigned char vals_to_write[], const uint32_t n_to_write) const {
	std::ofstream of(this->test_infile);
	for (uint32_t i = 0; i < n_to_write; ++i) {
	    of << vals_to_write[i];
	}
	of.close();
    }

    void run_test() {
    // Helper function for running the tests since only the data in test_infile differs.
	bxz::ifstream in(this->test_infile);
	std::string line;
	uint32_t i = 0;
	while (std::getline(in, line)) {
	    EXPECT_EQ(line[0], this->expected[i]);
	    ++i;
	}
	EXPECT_EQ(i, this->n_in_vals);
    }

};
uint32_t DecompressionTest::n_in_vals = 10;
std::vector<char> DecompressionTest::expected = std::vector<char>(10, '1');

#if defined(BXZSTR_Z_SUPPORT) && (BXZSTR_Z_SUPPORT) == 1
// Test z decompression
class ZDecompressionTest : public DecompressionTest, public ::testing::Test {
  protected:
    void SetUp() override {
	// Fake gzip data with 10 1s on their own lines (output of
	// `od -A x -t x1z -v` from a file with these contents).
	const unsigned char test_vals[] = {0x1f, 0x8b, 0x08, 0x08, 0xf1, 0x0a, 0x61, 0x62, 0x00, 0x03, 0x74, 0x65, 0x73, 0x74, 0x7a, 0x2e,
	                                   0x74, 0x78, 0x74, 0x00, 0x33, 0xe4, 0x32, 0xc4, 0x80, 0x00, 0x4c, 0xd2, 0xca, 0x03, 0x14, 0x00,
					   0x00, 0x00 };
	this->test_infile = "ZDecompressionTest_fake_data.txt.gz";
	this->write_test_data(test_vals, 34);
    }

};

#endif

#if defined(BXZSTR_BZ2_SUPPORT) && (BXZSTR_BZ2_SUPPORT) == 1
// Test bz decompression
class BzDecompressionTest : public DecompressionTest, public ::testing::Test {
  protected:
    void SetUp() override {
	// Fake bzip2 data with 10 1s on their own lines
	const unsigned char test_vals[] = { 0x42, 0x5a, 0x68, 0x39, 0x31, 0x41, 0x59, 0x26, 0x53, 0x59, 0x35, 0xaa, 0x83, 0x68, 0x00, 0x00,
	                                    0x09, 0xc8, 0x00, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x20, 0xa9, 0xa0, 0x82, 0x64, 0xce, 0x2e,
					    0xe4, 0x8a, 0x70, 0xa1, 0x20, 0x6b, 0x55, 0x06, 0xd0 };

	this->test_infile = "BzDecompressionTest_fake_data.txt.bz2";
	this->write_test_data(test_vals, 41);
    }

};

#endif

#if defined(BXZSTR_LZMA_SUPPORT) && (BXZSTR_LZMA_SUPPORT) == 1
// Test lzma decompression
class LzmaDecompressionTest : public DecompressionTest, public ::testing::Test {
  protected:
    void SetUp() override {
	// Fake xz data with 10 1s on their own lines
	const unsigned char test_vals[] = { 0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00, 0x00, 0x04, 0xe6, 0xd6, 0xb4, 0x46, 0x02, 0x00, 0x21, 0x01,
	                                    0x16, 0x00, 0x00, 0x00, 0x74, 0x2f, 0xe5, 0xa3, 0xe0, 0x00, 0x13, 0x00, 0x08, 0x5d, 0x00, 0x18,
					    0x82, 0xa7, 0x83, 0x80, 0x00, 0x00, 0x00, 0x00, 0xc3, 0x65, 0xdc, 0x8f, 0x27, 0x35, 0xda, 0x98,
					    0x00, 0x01, 0x24, 0x14, 0xf0, 0x80, 0xb4, 0xcb, 0x1f, 0xb6, 0xf3, 0x7d, 0x01, 0x00, 0x00, 0x00,
					    0x00, 0x04, 0x59, 0x5a };

	this->test_infile = "LzmaDecompressionTest_fake_data.txt.xz";
	this->write_test_data(test_vals, 68);
    }

};

#endif

#if defined(BXZSTR_ZSTD_SUPPORT) && (BXZSTR_ZSTD_SUPPORT) == 1
// Test zstd decompression
class ZstdDecompressionTest : public DecompressionTest, public ::testing::Test {
  protected:
    void SetUp() override {
	// Fake zstd data with 10 1s on their own lines
	const unsigned char test_vals[] = { 0x28, 0xb5, 0x2f, 0xfd, 0x00, 0x58, 0x45, 0x00, 0x00, 0x10, 0x31, 0x0a, 0x01, 0x00, 0x79, 0x0e,
	                                    0x0b, 0x28, 0xb5, 0x2f, 0xfd, 0x20, 0x00, 0x01, 0x00, 0x00 };
	this->test_infile = "ZstdDecompressionTest_fake_data.txt.zst";
	this->write_test_data(test_vals, 26);
    }

};

#endif

#endif
