/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#ifndef BXZSTR_BXZSTR_OFSTREAM_INTEGRATIONTEST_HPP
#define BXZSTR_BXZSTR_OFSTREAM_INTEGRATIONTEST_HPP

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "gtest/gtest.h"

#include "bxzstr.hpp"

// Integration tests for bxz::ofstream
//
// Define some vals that are used in all compression tests
class CompressionTest {
  protected:
    // Expected values
    std::vector<char> expected;

    // Test parameters
    std::string test_outfile;

    void write_test_data(const bxz::Compression compression) const {
	bxz::ofstream out(this->test_outfile, compression);
	for (uint32_t i = 0; i < this->n_out_vals; ++i) {
	    out << 1;
	    if (i < n_out_vals - 1)
		out << '\n';
	}
	out.flush();
    }

    void run_test() {
	// Helper function for running the tests since only the data in test_outfile differs.
	std::ifstream in(this->test_outfile);

	// Read in the whole input as a single string
	std::ostringstream oss;
	oss << in.rdbuf();
	std::string read_in = oss.str();
	uint32_t n_read_in = read_in.size();

	EXPECT_TRUE(this->expected.size() > 0); // Check that the write was successful.
	EXPECT_EQ(n_read_in, this->expected.size());
	for (uint32_t i = 0; i < n_read_in; ++i) {
	    EXPECT_EQ(read_in[i], this->expected[i]);
	}
    }

  private:
    static uint32_t n_out_vals;

};
uint32_t CompressionTest::n_out_vals = 10;

#if defined(BXZSTR_Z_SUPPORT) && (BXZSTR_Z_SUPPORT) == 1
// Test z compression
class ZCompressionTest : public CompressionTest, public ::testing::Test {
  protected:
    void SetUp() override {
	// Raw data from running bxz::ofstream with bxz::z for this test set
	const unsigned char test[] = { 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33, 0xe4, 0x32, 0x44, 0x87, 0x00,
	                               0xae, 0x30, 0x5a, 0x73, 0x13, 0x00, 0x00, 0x00, 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
				       0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	this->test_outfile = "ZCompressionTest_fake_data.txt.gz";
	this->write_test_data(bxz::z);
	for (uint32_t i = 0; i < sizeof(test)/sizeof(test[0]); ++i) {
	    expected.push_back(test[i]);
	}
    }

};

#endif

#if defined(BXZSTR_BZ2_SUPPORT) && (BXZSTR_BZ2_SUPPORT) == 1
// Test bz compression
class BzCompressionTest : public CompressionTest, public ::testing::Test {
  protected:
    void SetUp() override {
	// Raw data from running bxz::ofstream with bxz::bz2 for this test set
	const unsigned char test[] = { 0x42, 0x5a, 0x68, 0x36, 0x31, 0x41, 0x59, 0x26, 0x53, 0x59, 0x95, 0x99, 0x0f, 0x45, 0x00, 0x00,
	                               0x04, 0xc8, 0x00, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x20, 0xa9, 0xa0, 0x82, 0x65, 0x8e, 0x2e,
				       0xe4, 0x8a, 0x70, 0xa1, 0x21, 0x2b, 0x32, 0x1e, 0x8a, 0x42, 0x5a, 0x68, 0x36, 0x17, 0x72, 0x45,
				       0x38, 0x50, 0x90, 0x00, 0x00, 0x00, 0x00 };
	this->test_outfile = "BzCompressionTest_fake_data.txt.bz2";
	this->write_test_data(bxz::bz2);
	for (uint32_t i = 0; i < sizeof(test)/sizeof(test[0]); ++i) {
	    expected.push_back(test[i]);
	}
    }

};

#endif


#if defined(BXZSTR_LZMA_SUPPORT) && (BXZSTR_LZMA_SUPPORT) == 1
// Test lzma compression
class LzmaCompressionTest : public CompressionTest, public ::testing::Test {
  protected:
    void SetUp() override {
	// Raw data from running bxz::ofstream with bxz::lzma for this test set
	const unsigned char test[] = { 0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00, 0x00, 0x04, 0xe6, 0xd6, 0xb4, 0x46, 0x02, 0x00, 0x21, 0x01,
	                               0x16, 0x00, 0x00, 0x00, 0x74, 0x2f, 0xe5, 0xa3, 0xe0, 0x00, 0x12, 0x00, 0x07, 0x5d, 0x00, 0x18,
				       0x82, 0xa7, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0x5f, 0xfe, 0x41, 0x33, 0xb3, 0xaf, 0x49,
				       0x00, 0x01, 0x23, 0x13, 0x94, 0x83, 0x91, 0x1a, 0x1f, 0xb6, 0xf3, 0x7d, 0x01, 0x00, 0x00, 0x00,
				       0x00, 0x04, 0x59, 0x5a, 0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00, 0x00, 0x04, 0xe6, 0xd6, 0xb4, 0x46,
				       0x00, 0x00, 0x00, 0x00, 0x1c, 0xdf, 0x44, 0x21, 0x1f, 0xb6, 0xf3, 0x7d, 0x01, 0x00, 0x00, 0x00,
				       0x00, 0x04, 0x59, 0x5a };

	this->test_outfile = "LzmaCompressionTest_fake_data.txt.xz";
	this->write_test_data(bxz::lzma);
	for (uint32_t i = 0; i < sizeof(test)/sizeof(test[0]); ++i) {
	    expected.push_back(test[i]);
	}
    }

};

#endif


#if defined(BXZSTR_ZSTD_SUPPORT) && (BXZSTR_ZSTD_SUPPORT) == 1
// Test zstd compression
class ZstdCompressionTest : public CompressionTest, public ::testing::Test {
  protected:
    void SetUp() override {
	// Raw data from running bxz::ofstream with bxz::lzma for this test set
	const unsigned char test[] = { 0x28, 0xb5, 0x2f, 0xfd, 0x00, 0x58, 0x45, 0x00, 0x00, 0x10, 0x31, 0x0a, 0x01, 0x00, 0xcd, 0x0e,
	                               0x0b, 0x28, 0xb5, 0x2f, 0xfd, 0x20, 0x00, 0x01, 0x00, 0x00 };

	this->test_outfile = "ZstdCompressionTest_fake_data.txt.zst";
	this->write_test_data(bxz::zstd);
	for (uint32_t i = 0; i < sizeof(test)/sizeof(test[0]); ++i) {
	    expected.push_back(test[i]);
	}
    }

};

#endif

#endif
