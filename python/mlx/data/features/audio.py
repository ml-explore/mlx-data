# Copyright © 2023 Apple Inc.

import enum
import math

import numpy as np


class WindowType(enum.Enum):
    """Enum to choose the window function."""

    Hamming = 0
    """Hamming window implemented with :func:`numpy.hamming`."""

    Hanning = 1
    """Hanning window implemented with :func:`numpy.hanning`."""


class FrequencyScale(enum.Enum):
    """Enum to choose the frequency scaling for the filter banks."""

    MEL = 0
    """The commonly used Mel scale."""

    LOG10 = 1
    """Logarithmic scale."""

    LINEAR = 2
    """No scaling."""


def next_pow_2(n):
    return 2 ** math.ceil(math.log2(n))


def num_sample_per_frame(sampling_freq, frame_size_ms):
    return round(sampling_freq * frame_size_ms / 1000.0)


def hertz_to_warped_scale(hz, freqscale):
    if freqscale == FrequencyScale.MEL:
        return 2595.0 * np.log10(1.0 + hz / 700.0)
    elif freqscale == FrequencyScale.LOG10:
        return np.log10(hz)
    elif freqscale == FrequencyScale.LINEAR:
        return hz
    else:
        raise ValueError("invalid freqscale")


def warped_to_hertz_scale(wrp, freqscale):
    if freqscale == FrequencyScale.MEL:
        return 700.0 * (10 ** (wrp / 2595.0) - 1)
    elif freqscale == FrequencyScale.LOG10:
        return np.pow(10, wrp)
    elif freqscale == FrequencyScale.LINEAR:
        return wrp
    else:
        raise ValueError("invalid freqscale")


def sliding_window(axis, window, stride):
    def sliding_window_impl(x):
        if axis < 0:
            paxis = x.ndim + axis
        else:
            paxis = axis
        isz = x.shape[axis]
        if isz < window:
            osz = 0
        else:
            osz = (isz - window) // stride + 1
        windows = (
            x.shape[:paxis]
            + (
                osz,
                window,
            )
            + x.shape[paxis + 1 :]
        )
        strides = x.strides[:paxis] + (x.strides[paxis] * stride,) + x.strides[paxis:]
        res = np.lib.stride_tricks.as_strided(x, windows, strides)

        return res

    return sliding_window_impl


def pre_emphasis(coeff=1.0):
    def pre_emphasis_impl(x):
        xm1 = np.roll(x, 1, -1)
        xm1[..., 0] = x[..., 0]
        res = x - coeff * xm1
        return res

    return pre_emphasis_impl


def windowing(window, windowtype):
    if windowtype == WindowType.Hamming:
        coeffs = np.hamming(window)
    elif windowtype == WindowType.Hanning:
        coeffs = np.hanning(window)
    else:
        raise ValueError("invalid windowtype")
    coeffs = coeffs.astype(np.float32)

    def windowing_impl(x):
        res = x * coeffs
        return res

    return windowing_impl


def power_spectrum(n_fft):
    def power_spectrum_impl(x):
        return np.abs(np.fft.rfft(x, n_fft)).astype(x.dtype)

    return power_spectrum_impl


def tri_filterbank(
    numfilters,
    filterlen,
    samplingfreq,
    lowfreq=0,
    highfreq=-1,
    melfloor=0.0,
    freqscale=FrequencyScale.MEL,
):
    if highfreq <= 0:
        highfreq = samplingfreq // 2

    minwarpfreq = hertz_to_warped_scale(lowfreq, freqscale)
    maxwarpfreq = hertz_to_warped_scale(highfreq, freqscale)
    dwarp = (maxwarpfreq - minwarpfreq) / (numfilters + 1)

    f = np.arange(numfilters + 2)
    f = (
        warped_to_hertz_scale(f * dwarp + minwarpfreq, freqscale)
        * (filterlen - 1.0)
        * 2.0
        / samplingfreq
    )

    hislope = np.arange(filterlen)[:, None] - f[None, :]
    hislope = hislope / (np.roll(f, -1) - f)
    hislope = hislope[:, :numfilters]

    loslope = np.roll(f, -2)[None, :] - np.arange(filterlen)[:, None]
    loslope = loslope / (np.roll(f, -2) - np.roll(f, -1))
    loslope = loslope[:, :numfilters]

    H = np.maximum(np.minimum(hislope, loslope), 0.0).astype(np.float32)

    def tri_filterbank_impl(x):
        return np.maximum(x @ H, melfloor)

    return tri_filterbank_impl


def mfsc(
    n_filterbank,
    sampling_freq,
    frame_size_ms=25,
    frame_stride_ms=10,
    pre_emphasis_coeff=0.97,
    window_type=WindowType.Hamming,
    low_freq=0,
    high_freq=-1,
    mel_floor=1.0,
    freq_scale=FrequencyScale.MEL,
    post_process=None,
):
    """Returns a function that computes spectrogram features from audio in
    particular mel-frequency spectral coefficients (MFSCs).

    .. note::
       This feature extractor operates on mono audio provided as a 1D array.
       Meaning the input should have shape ``(N,)`` and not ``(N, 1)``. You may
       want to look into :meth:`~mlx.data.Buffer.squeeze` to transform arrays of shape
       ``(N, 1)`` to ``(N,)``.

    The featurization function

    1. computes a sliding window of the input audio
    2. applies a pre-emphasis filter
    3. applies a windowing function
    4. computes the power spectrum
    5. computes triangular filterbank features
    6. compute the log of the features
    7. apply any post processing function that may be provided

    The following example loads the librispeech dataset and computes MFSC
    features:

    .. code-block:: python

        from mlx.data.datasets import load_librispeech
        from mlx.data.features import mfsc

        dset = (
            load_librispeech()
            .squeeze("audio")
            .key_transform("audio", mfsc(80, 16000))
            .to_stream()
            .prefetch(16, 8)
            .batch(16)
            .prefetch(2, 1)
        )

    Args:
        n_filterbank (int): How many frequency bands to use. This number will
            be the dimensionality of the resulting features.
        sampling_freq (int): The sampling frequency of the input audio in Hz.
        frame_size_ms (int): Each output feature will correspond to that many
            milliseconds of input audio. (default: 25)
        frame_stride_ms (int): Two consecutive features will correspond to
            audio windows that are that many milliseconds apart. (default: 10)
        pre_emphasis_coeff (float): Defines the free parameter of the FIR
            filter that does the pre-emphasis. (default: 0.97)
        window_type (WindowType): Defines the windowing function to use before
            computing the power spectrum. (default: WindowType.Hamming)
        low_freq (int): The lowest frequency to use when creating the frequency
            bands. Simply put, signal power in lower frequencies is ignored.
            (default: 0)
        high_freq (int): The highest frequency to use when creating the frequency
            bands. Simply put, signal power in higher frequencies is ignored.
            If set to -1 then ``sampling_freq // 2`` is used.  (default: -1)
        mel_floor (float): The minimum power collected in the filterbanks. Even
            though the name is ``mel_floor`` this applies even when using different
            frequency scales. (default: 1.0)
        freq_scale (FrequencyScale): The frequency scale to use when computing
            the frequency bands for the filterbanks. (default: FrequencyScale.MEL)
        post_process (callable, optional): An optional callable to post process
            the MFSC features. (default: None)
    """
    n_sample_per_frame = num_sample_per_frame(sampling_freq, frame_size_ms)
    n_sample_per_stride = num_sample_per_frame(sampling_freq, frame_stride_ms)
    slwin = sliding_window(-1, n_sample_per_frame, n_sample_per_stride)
    n_fft = next_pow_2(n_sample_per_frame)
    pe = pre_emphasis(pre_emphasis_coeff)
    win = windowing(n_sample_per_frame, window_type)
    ps = power_spectrum(n_fft)
    tfb = tri_filterbank(
        n_filterbank,
        n_fft // 2 + 1,
        sampling_freq,
        low_freq,
        high_freq,
        mel_floor,
        freq_scale,
    )

    def mfsc_impl(x):
        out = x * 32768.0
        out = slwin(out)
        out = pe(out)
        out = win(out)
        out = ps(out)
        out = tfb(out)
        out = np.log(np.maximum(out, np.finfo(np.float32).tiny))
        if post_process is not None:
            out = post_process(out)
        return out

    return mfsc_impl
