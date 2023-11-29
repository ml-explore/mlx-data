/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "compression_types_unittest.hpp"

#include <cstddef>

#if defined(BXZSTR_Z_SUPPORT) && (BXZSTR_Z_SUPPORT) == 1
// bxz::z test
TEST_F(DetectZTest, GzipAndZlibHeadersReturnZ) {
    for (size_t i = 0; i < this->test_headers.size(); ++i) {
	const bxz::Compression &got = bxz::detect_type(reinterpret_cast<char*>(&this->test_headers[i][0]), reinterpret_cast<char*>(&this->test_headers[i][this->test_headers[i].size()]));
	EXPECT_EQ(got, expected);
    }
}

TEST(BxzRunTest, BxzRunReturnsZNoFlush) {
    const int got = bxz_run(bxz::z);
    EXPECT_EQ(got, Z_NO_FLUSH);
}

TEST(BxzFinishTest, BxzFinishReturnsZFinish) {
    const int got = bxz_finish(bxz::z);
    EXPECT_EQ(got, Z_FINISH);
}

#endif

#if defined(BXZSTR_BZ2_SUPPORT) && (BXZSTR_BZ2_SUPPORT) == 1
// bxz::bz2 test
TEST_F(DetectBz2Test, BzHeaderReturnsBz2) {
    for (size_t i = 0; i < this->test_headers.size(); ++i) {
	const bxz::Compression &got = bxz::detect_type(reinterpret_cast<char*>(&this->test_headers[i][0]), reinterpret_cast<char*>(&this->test_headers[i][this->test_headers[i].size()]));
	EXPECT_EQ(got, expected);
    }
}

TEST(BxzRunTest, BxzRunReturnsBzRun) {
    const int got = bxz_run(bxz::bz2);
    EXPECT_EQ(got, BZ_RUN);
}

TEST(BxzFinishTest, BxzFinishReturnsBzFinish) {
    const int got = bxz_finish(bxz::bz2);
    EXPECT_EQ(got, BZ_FINISH);
}

#endif

#if defined(BXZSTR_LZMA_SUPPORT) && (BXZSTR_LZMA_SUPPORT) == 1
// bxz::lzma test
TEST_F(DetectLzmaTest, LzmaHeaderReturnsLzma) {
    for (size_t i = 0; i < this->test_headers.size(); ++i) {
	const bxz::Compression &got = bxz::detect_type(reinterpret_cast<char*>(&this->test_headers[i][0]), reinterpret_cast<char*>(&this->test_headers[i][this->test_headers[i].size()]));
	EXPECT_EQ(got, expected);
    }
}

TEST(BxzRunTest, BxzRunReturnsLzmaRun) {
    const int got = bxz_run(bxz::lzma);
    EXPECT_EQ(got, LZMA_RUN);
}

TEST(BxzFinishTest, BxzFinishReturnsLzmaFinish) {
    const int got = bxz_finish(bxz::lzma);
    EXPECT_EQ(got, LZMA_FINISH);
}

#endif

#if defined(BXZSTR_ZSTD_SUPPORT) && (BXZSTR_ZSTD_SUPPORT) == 1
// bxz::zstd test
TEST_F(DetectZstdTest, ZstdHeaderReturnsZstd) {
    for (size_t i = 0; i < this->test_headers.size(); ++i) {
	const bxz::Compression &got = bxz::detect_type(reinterpret_cast<char*>(&this->test_headers[i][0]), reinterpret_cast<char*>(&this->test_headers[i][this->test_headers[i].size()]));
	EXPECT_EQ(got, expected);
    }
}

TEST(BxzRunTest, BxzRunReturnsZstd0) {
    const int got = bxz_run(bxz::zstd);
    EXPECT_EQ(got, 0);
}

TEST(BxzFinishTest, BxzFinishReturnsZstdEndStream) {
    const int got = bxz_finish(bxz::zstd);
    EXPECT_EQ(got, 1);
}

#endif

