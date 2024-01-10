# Copyright © 2023 Apple Inc.

import argparse
from pathlib import Path

import torch
from sentencepiece import SentencePieceProcessor
from torchaudio.datasets import LIBRISPEECH
from torchaudio.transforms import MelSpectrogram
from utils import Benchmark


class TransformedLibrispeech(LIBRISPEECH):
    def __init__(self, root, tokenizer):
        root = Path(root)
        if root.name == "dev-clean":
            super().__init__(root.parent.parent, "dev-clean")
        else:
            super().__init__(root, "dev-clean")

        self.transform = MelSpectrogram(
            sample_rate=16000, n_fft=512, win_length=400, hop_length=160, n_mels=128
        )
        self.tokenizer = tokenizer

    def __getitem__(self, idx):
        audio, _, transcript, _, _, _ = super().__getitem__(idx)

        audio = self.transform(audio.squeeze())
        transcript = self.tokenizer.encode(transcript.lower())
        transcript = [self.tokenizer.bos_id()] + transcript + [self.tokenizer.eos_id()]

        return (audio, transcript)

    @staticmethod
    def collate_fn(samples):
        audio, transcript = zip(*samples)
        transcript_lengths = list(map(len, transcript))
        N = max(transcript_lengths)
        transcript = [t + [0] * (N - len(t)) for t in transcript]
        transcript = torch.tensor(transcript)

        audio_lengths = [a.shape[1] for a in audio]
        N = max(audio_lengths)
        audio_batched = torch.zeros((len(audio), len(audio[0]), N))
        for i, a in enumerate(audio):
            audio_batched[i, :, : a.shape[1]] = a

        return {
            "audio": audio_batched,
            "audio_length": torch.tensor(audio_lengths),
            "transcript": transcript,
            "transcript_length": torch.tensor(transcript_lengths),
        }


def iterate(args, workers):
    tokenizer = SentencePieceProcessor(args.tokenizer_file)
    dset = TransformedLibrispeech(args.data_dir, tokenizer)
    data_loader = torch.utils.data.DataLoader(
        dset,
        shuffle=True,
        batch_size=args.batch_size,
        num_workers=workers,
        collate_fn=TransformedLibrispeech.collate_fn,
    )

    cnt = 0
    for batch in data_loader:
        cnt += 1
    return cnt


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("data_dir")
    parser.add_argument("--batch_size", type=int, default=32)
    parser.add_argument("--tokenizer_file", default="tokenizer.model")
    args = parser.parse_args()

    benchmark = Benchmark("PyTorch Librispeech")
    for i in range(3):
        benchmark.log_run("iterate_no_workers", iterate, args, 0)

    for i in range(3):
        benchmark.log_run("iterate_workers_8", iterate, args, 8)

    for i in range(3):
        benchmark.log_run("iterate_workers_16", iterate, args, 16)
    benchmark.report()
