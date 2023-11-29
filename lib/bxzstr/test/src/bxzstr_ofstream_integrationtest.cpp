/* This Source Code Form is subject to the terms of the Mozilla Public
g * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "bxzstr_ofstream_integrationtest.hpp"

#if defined(BXZSTR_Z_SUPPORT) && (BXZSTR_Z_SUPPORT) == 1
// Test Z Compression
TEST_F(ZCompressionTest, BxzOfstreamCompressesZ) {
    this->run_test();
}

#endif

#if defined(BXZSTR_BZ2_SUPPORT) && (BXZSTR_BZ2_SUPPORT) == 1
// Test Bz2 Compression
TEST_F(BzCompressionTest, BxzIfstreamCompressesBz) {
    this->run_test();
}

#endif

#if defined(BXZSTR_LZMA_SUPPORT) && (BXZSTR_LZMA_SUPPORT) == 1
// Test Lzma Compression
TEST_F(LzmaCompressionTest, BxzIfstreamCompressesLzma) {
    this->run_test();
}

#endif

#if defined(BXZSTR_ZSTD_SUPPORT) && (BXZSTR_ZSTD_SUPPORT) == 1
// Test Zstd Compression
TEST_F(ZstdCompressionTest, BxzIfstreamCompressesZstd) {
    this->run_test();
}

#endif
