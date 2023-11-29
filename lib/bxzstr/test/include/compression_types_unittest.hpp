/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#ifndef BXZSTR_COMPRESSION_TYPES_UNITTEST_HPP
#define BXZSTR_COMPRESSION_TYPES_UNITTEST_HPP

#include <array>
#include <vector>

#include "gtest/gtest.h"

#include "bxzstr.hpp"

// Test detect_type
//
#if defined(BXZSTR_Z_SUPPORT) && (BXZSTR_Z_SUPPORT) == 1
// Detect bxz::z
class DetectZTest : public ::testing::Test {
  protected:
    void SetUp() {
	test_headers.emplace_back(std::array<unsigned char, 2>({ 0x1F, 0x8B }));
	test_headers.emplace_back(std::array<unsigned char, 2>({ 0x78, 0x01 }));
	test_headers.emplace_back(std::array<unsigned char, 2>({ 0x78, 0x9C }));
	test_headers.emplace_back(std::array<unsigned char, 2>({ 0x78, 0xDA }));
    }
    void TearDown() {
	test_headers.clear();
	test_headers.shrink_to_fit();
    }
    // Test input
    std::vector<std::array<unsigned char, 2>> test_headers;
    // Expected
    bxz::Compression expected = bxz::z;
};

#endif

#if defined(BXZSTR_BZ2_SUPPORT) && (BXZSTR_BZ2_SUPPORT) == 1
// Detect bxz::bz2
class DetectBz2Test : public ::testing::Test {
  protected:
    void SetUp() {
	test_headers.emplace_back(std::array<unsigned char, 3>({ 0x42, 0x5a, 0x68 }));
    }
    void TearDown() {
	test_headers.clear();
	test_headers.shrink_to_fit();
    }
    // Test input
    std::vector<std::array<unsigned char, 3>> test_headers;
    // Expected
    bxz::Compression expected = bxz::bz2;
};

#endif

#if defined(BXZSTR_LZMA_SUPPORT) && (BXZSTR_LZMA_SUPPORT) == 1
// Detect bxz::lzma
class DetectLzmaTest : public ::testing::Test {
  protected:
    void SetUp() {
	test_headers.emplace_back(std::array<unsigned char, 6>({ 0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00 }));
    }
    void TearDown() {
	test_headers.clear();
	test_headers.shrink_to_fit();
    }
    // Test input
    std::vector<std::array<unsigned char, 6>> test_headers;
    // Expected
    bxz::Compression expected = bxz::lzma;
};

#endif

#if defined(BXZSTR_ZSTD_SUPPORT) && (BXZSTR_ZSTD_SUPPORT) == 1
// Detect bxz::lzma
class DetectZstdTest : public ::testing::Test {
  protected:
    void SetUp() {
	test_headers.emplace_back(std::array<unsigned char, 4>({ 0x28, 0xB5, 0x2F, 0xFD }));
    }
    void TearDown() {
	test_headers.clear();
	test_headers.shrink_to_fit();
    }
    // Test input
    std::vector<std::array<unsigned char, 4>> test_headers;
    // Expected
    bxz::Compression expected = bxz::zstd;
};

#endif

// Return plaintext if no header is identified
class DetectTypeTest : public ::testing::Test {
  protected:
    void SetUp() {
	test_headers.emplace_back(std::array<unsigned char, 4>({ 0x00, 0x00, 0x00, 0x00 }));
    }
    void TearDown() {
	test_headers.clear();
	test_headers.shrink_to_fit();
    }
    // Test input
    std::vector<std::array<unsigned char, 4>> test_headers;
    // Expected
    bxz::Compression expected = bxz::plaintext;
};

#endif
