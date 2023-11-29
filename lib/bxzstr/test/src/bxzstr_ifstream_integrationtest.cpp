/* This Source Code Form is subject to the terms of the Mozilla Public
g * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "bxzstr_ifstream_integrationtest.hpp"

#if defined(BXZSTR_Z_SUPPORT) && (BXZSTR_Z_SUPPORT) == 1
// Test Z Decompression
TEST_F(ZDecompressionTest, BxzIfstreamDecompressesZ) {
    this->run_test();
}

#endif

#if defined(BXZSTR_BZ2_SUPPORT) && (BXZSTR_BZ2_SUPPORT) == 1
// Test Bz2 Decompression
TEST_F(BzDecompressionTest, BxzIfstreamDecompressesBz) {
    this->run_test();
}

#endif

#if defined(BXZSTR_LZMA_SUPPORT) && (BXZSTR_LZMA_SUPPORT) == 1
// Test Lzma Decompression
TEST_F(LzmaDecompressionTest, BxzIfstreamDecompressesLzma) {
    this->run_test();
}

#endif

#if defined(BXZSTR_ZSTD_SUPPORT) && (BXZSTR_ZSTD_SUPPORT) == 1
// Test Zstd Decompression
TEST_F(ZstdDecompressionTest, BxzIfstreamDecompressesZstd) {
    this->run_test();
}

#endif
