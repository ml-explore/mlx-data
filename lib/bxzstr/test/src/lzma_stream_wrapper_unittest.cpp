/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "bxzstr.hpp"

#if defined(BXZSTR_LZMA_SUPPORT) && (BXZSTR_LZMA_SUPPORT) == 1

#include "lzma_stream_wrapper_unittest.hpp"

TEST_F(LzmaExceptionTest, MsgConstructorWorks) {
    bxz::lzmaException e(msgConstructorValue);
    const std::string &got = e.what();
    EXPECT_EQ(msgConstructorExpected, got);
}

TEST_F(LzmaExceptionTest, ErrcodeConstructorWorks) {
    for (size_t i = 0; i < errcodeInputs.size(); ++i) {
	bxz::lzmaException e(errcodeInputs.at(i));
	const std::string &got = e.what();
	EXPECT_EQ(errcodeExpecteds.at(i), got);
    }
}

TEST_F(LzmaStreamWrapperTest, ConstructorDoesNotThrowOnInput) {
    EXPECT_NO_THROW(bxz::detail::lzma_stream_wrapper wrapper(testTrue));
}

TEST_F(LzmaStreamWrapperTest, ConstructorDoesNotThrowOnOutput) {
    EXPECT_NO_THROW(bxz::detail::lzma_stream_wrapper wrapper(testFalse));
}

TEST_F(LzmaDecompressTest, DecompressDoesNotThrowOnValidInput) {
    EXPECT_NO_THROW(wrapper->decompress());
}

TEST_F(LzmaDecompressTest, DecompressThrowsOnInvalidInput) {
    testIn[0] = 0x1d;
    EXPECT_THROW(wrapper->decompress(), bxz::lzmaException);
}

TEST_F(LzmaDecompressTest, DecompressUpdatesStreamState) {
    wrapper->decompress();
    EXPECT_EQ(wrapper->next_in(), &testIn[10]);
    EXPECT_EQ(wrapper->avail_in(), 0);
    EXPECT_EQ(wrapper->next_out(), &testOut[0]);
    EXPECT_EQ(wrapper->avail_out(), 10);
}

TEST_F(LzmaCompressTest, CompressEndsStream) {
    wrapper->set_avail_out(0);
    wrapper->set_next_out(&testOut[10]);
    EXPECT_NO_THROW(wrapper->compress(true));
}

TEST_F(LzmaCompressTest, CompressDoesNotThrowOnValidInput) {
    EXPECT_NO_THROW(wrapper->compress(false));
}

TEST_F(LzmaCompressTest, CompressUpdatesStreamState) {
    wrapper->compress(false);
    EXPECT_EQ(wrapper->next_in(), &testIn[0]);
    EXPECT_EQ(wrapper->avail_in(), 10);
    EXPECT_EQ(wrapper->next_out(), &testOut[10]);
    EXPECT_EQ(wrapper->avail_out(), 0);
}

#endif
