/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "bxzstr.hpp"

#if defined(BXZSTR_BZ2_SUPPORT) && (BXZSTR_BZ2_SUPPORT) == 1

#include "bz_stream_wrapper_unittest.hpp"

TEST_F(BzExceptionTest, MsgConstructorWorks) {
    bxz::bzException e(msgConstructorValue);
    const std::string &got = e.what();
    EXPECT_EQ(msgConstructorExpected, got);
}

TEST_F(BzExceptionTest, ErrcodeConstructorWorks) {
    for (size_t i = 0; i < errcodeInputs.size(); ++i) {
	bxz::bzException e(errcodeInputs.at(i));
	const std::string &got = e.what();
	EXPECT_EQ(errcodeExpecteds.at(i), got);
    }
}

TEST_F(BzStreamWrapperTest, ConstructorDoesNotThrowOnInput) {
    EXPECT_NO_THROW(bxz::detail::bz_stream_wrapper wrapper(testTrue));
}

TEST_F(BzStreamWrapperTest, ConstructorDoesNotThrowOnOutput) {
    EXPECT_NO_THROW(bxz::detail::bz_stream_wrapper wrapper(testFalse));
}

TEST_F(BzDecompressTest, DecompressDoesNotThrowOnValidInput) {
    EXPECT_NO_THROW(wrapper->decompress());
}

TEST_F(BzDecompressTest, DecompressThrowsOnInvalidInput) {
    testIn[0] = 0x1d;
    EXPECT_THROW(wrapper->decompress(), bxz::bzException);
}

TEST_F(BzDecompressTest, DecompressUpdatesStreamState) {
    wrapper->decompress();
    EXPECT_EQ(wrapper->next_in(), &testIn[10]);
    EXPECT_EQ(wrapper->avail_in(), 0);
    EXPECT_EQ(wrapper->next_out(), &testOut[0]);
    EXPECT_EQ(wrapper->avail_out(), 10);
}

TEST_F(BzCompressTest, CompressEndsStream) {
    wrapper->set_avail_out(0);
    wrapper->set_next_out(&testOut[10]);
    EXPECT_NO_THROW(wrapper->compress(true));
}

TEST_F(BzCompressTest, CompressDoesNotThrowOnValidInput) {
    EXPECT_NO_THROW(wrapper->compress(false));
}

TEST_F(BzCompressTest, CompressUpdatesStreamState) {
    wrapper->compress(false);
    EXPECT_EQ(wrapper->next_in(), &testIn[10]);
    EXPECT_EQ(wrapper->avail_in(), 0);
    EXPECT_EQ(wrapper->next_out(), &testOut[0]);
    EXPECT_EQ(wrapper->avail_out(), 10);
}

#endif
