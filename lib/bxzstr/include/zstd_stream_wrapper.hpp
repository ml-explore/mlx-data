/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#if defined(BXZSTR_ZSTD_SUPPORT) && (BXZSTR_ZSTD_SUPPORT) == 1

#ifndef BXZSTR_ZSTD_STREAM_WRAPPER_HPP
#define BXZSTR_ZSTD_STREAM_WRAPPER_HPP

#include <zstd.h>

#include <string>
#include <exception>

#include "stream_wrapper.hpp"

namespace bxz {
/// Exception class thrown by failed zstd operations.
class zstdException : public std::exception {
  public:
    zstdException(const size_t err) : msg("zstd error: ") {
	this->msg += "[" + std::to_string(err) + "]: ";
        this->msg += ZSTD_getErrorName(err);
    }
    zstdException(const std::string _msg) : msg(_msg) {}

    const char * what() const noexcept { return this->msg.c_str(); }

  private:
    std::string msg;

}; // class zstdException

namespace detail {
class zstd_stream_wrapper : public stream_wrapper {
  public:
    zstd_stream_wrapper(const bool _isInput = true,
			const int level = ZSTD_defaultCLevel(), const int = 0)
	    : isInput(_isInput) {
	if (this->isInput) {
	    this->dctx = ZSTD_createDCtx();
	    if (this->dctx == NULL) throw zstdException("ZSTD_createDCtx() failed!");
	} else {
	    this->cctx = ZSTD_createCCtx();
	    if (this->cctx == NULL) throw zstdException("ZSTD_createCCtx() failed!");
	    this->ret = ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, level);
	}
	if (ZSTD_isError(this->ret)) throw zstdException(this->ret);

    }

    ~zstd_stream_wrapper() {
	if (this->isInput) {
	    ZSTD_freeDCtx(this->dctx);
	} else {
	    ZSTD_freeCCtx(this->cctx);
	}
    }

    int decompress(const int = 0) override {
	this->update_inbuffer();
	this->update_outbuffer();

	this->ret = ZSTD_decompressStream(this->dctx, &output, &input);
	if (ZSTD_isError(this->ret)) throw zstdException(this->ret);

	this->update_stream_state();

	return (int)ret;
    }

    int compress(const int endStream) override {
	this->update_inbuffer();
	this->update_outbuffer();

	if (endStream) {
	    this->ret = ZSTD_endStream(this->cctx, &output);
	    if (ZSTD_isError(this->ret)) throw zstdException(this->ret);
	} else {
	    this->ret = ZSTD_compressStream2(this->cctx, &output, &input, ZSTD_e_continue);
	    if (ZSTD_isError(this->ret)) throw zstdException(this->ret);

	    this->ret = (input.pos == input.size);
	}

	this->update_stream_state();

	return (int)ret;
    }

    bool stream_end() const override { return this->ret == 0; }
    bool done() const override { return this->stream_end(); }

    const unsigned char* next_in() const override { return static_cast<unsigned char*>(this->buffIn); }
    long avail_in() const override { return this->buffInSize; }
    unsigned char* next_out() const override { return static_cast<unsigned char*>(this->buffOut); }
    long avail_out() const override { return this->buffOutSize; }

    void set_next_in(const unsigned char* in) override { this->buffIn = (void*)in; }
    void set_avail_in(long in) override { this->buffInSize = (size_t)in; }
    void set_next_out(const unsigned char* in) override { this->buffOut = (void*)in; }
    void set_avail_out(long in) override { this->buffOutSize = (size_t)in; }

  private:
    bool isInput;
    size_t ret;

    size_t buffInSize;
    void* buffIn;
    size_t buffOutSize;
    void* buffOut;

    ZSTD_DCtx* dctx;
    ZSTD_CCtx* cctx;

    ZSTD_inBuffer input;
    ZSTD_outBuffer output;

    void update_inbuffer() { this->input = { this->buffIn, this->buffInSize, 0 }; }
    void update_outbuffer() { this->output =  { this->buffOut, this->buffOutSize, 0 }; }
    void update_stream_state() {
	this->set_next_out(this->next_out() + this->output.pos);
	this->set_avail_out(this->avail_out() - this->output.pos);
	this->set_next_in(this->next_in() + this->input.pos);
	this->set_avail_in(this->avail_in() - this->input.pos);
    }

}; // class zstd_stream_wrapper
} // namespace detail
} // namespace bxz

#endif
#endif
