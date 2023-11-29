/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "bxzstr.hpp"

#if defined(BXZSTR_Z_SUPPORT) && (BXZSTR_Z_SUPPORT) == 1

#include "z_stream_wrapper_unittest.hpp"

TEST_F(ZExceptionTest, MsgConstructorWorks) {
    bxz::zException e(msgConstructorValue);
    const std::string &got = e.what();
    EXPECT_EQ(msgConstructorExpected, got);
}

TEST_F(ZExceptionTest, ErrcodeConstructorWorks) {
    for (size_t i = 0; i < errcodeInputs.size(); ++i) {
	bxz::zException e(msgConstructorValue, errcodeInputs.at(i));
	const std::string &got = e.what();
	EXPECT_EQ(errcodeExpecteds.at(i), got);
    }
}

TEST_F(ZStreamWrapperTest, ConstructorDoesNotThrowOnInput) {
    EXPECT_NO_THROW(bxz::detail::z_stream_wrapper wrapper(testTrue));
}

TEST_F(ZStreamWrapperTest, ConstructorDoesNotThrowOnOutput) {
    EXPECT_NO_THROW(bxz::detail::z_stream_wrapper wrapper(testFalse));
}

TEST_F(ZDecompressTest, DecompressDoesNotThrowOnValidInput) {
    EXPECT_NO_THROW(wrapper->decompress());
}

TEST_F(ZDecompressTest, DecompressThrowsOnInvalidInput) {
    testIn[0] = 0x1d;
    EXPECT_THROW(wrapper->decompress(), bxz::zException);
}

TEST_F(ZDecompressTest, DecompressUpdatesStreamState) {
    wrapper->decompress();
    EXPECT_EQ(wrapper->next_in(), &testIn[10]);
    EXPECT_EQ(wrapper->avail_in(), 0);
    EXPECT_EQ(wrapper->next_out(), &testOut[0]);
    EXPECT_EQ(wrapper->avail_out(), 10);
}

TEST_F(ZCompressTest, CompressEndsStream) {
    wrapper->set_avail_out(0);
    wrapper->set_next_out(&testOut[10]);
    EXPECT_NO_THROW(wrapper->compress(true));
}

TEST_F(ZCompressTest, CompressDoesNotThrowOnValidInput) {
    EXPECT_NO_THROW(wrapper->compress(false));
}

TEST_F(ZCompressTest, CompressUpdatesStreamState) {
    wrapper->compress(false);
    EXPECT_EQ(wrapper->next_in(), &testIn[10]);
    EXPECT_EQ(wrapper->avail_in(), 0);
    EXPECT_EQ(wrapper->next_out(), &testOut[10]);
    EXPECT_EQ(wrapper->avail_out(), 0);
}

#endif
