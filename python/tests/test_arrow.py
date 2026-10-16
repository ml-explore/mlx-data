# Copyright © 2026 Apple Inc.

import unittest

import mlx.data as dx

try:
    import numpy as np
    import pyarrow as pa

    HAS_ARROW = True
except ImportError:
    HAS_ARROW = False


@unittest.skipIf(not HAS_ARROW, "pyarrow is not installed")
class TestArrow(unittest.TestCase):
    def test_values_and_keys(self):
        t = pa.table(
            {
                "a": pa.array([1, 2, 3, 4], pa.int64()),
                "b": pa.array([1.5, 2.5, 3.5, 4.5], pa.float32()),
                "c": pa.array([7, 8, 9, 10], pa.int32()),
            }
        )
        s = next(dx.stream_arrow(t))
        self.assertEqual(set(s.keys()), {"a", "b", "c"})
        # One sample per record batch: the leading dimension is the batch size, not 1.
        self.assertEqual(s["a"].shape, (4,))
        self.assertTrue(np.array_equal(s["a"], [1, 2, 3, 4]))
        self.assertTrue(np.array_equal(s["b"], [1.5, 2.5, 3.5, 4.5]))
        self.assertTrue(np.array_equal(s["c"], [7, 8, 9, 10]))
        self.assertEqual(s["a"].dtype, np.int64)
        self.assertEqual(s["b"].dtype, np.float32)
        self.assertEqual(s["c"].dtype, np.int32)

    def test_one_sample_per_batch(self):
        t = pa.table({"a": pa.array(range(10), pa.int64())})
        batches = list(dx.stream_arrow(t.to_reader(max_chunksize=4)))
        self.assertEqual([b["a"].shape[0] for b in batches], [4, 4, 2])
        self.assertTrue(
            np.array_equal(np.concatenate([b["a"] for b in batches]), range(10))
        )

    def test_sliced_batch_honours_offset(self):
        # A slice does not copy: it moves Arrow's `offset`. Reading from the buffer's start
        # instead would silently return the wrong rows.
        t = pa.table({"a": pa.array(range(10), pa.int64())})
        s = next(dx.stream_arrow(t.slice(3, 4)))
        self.assertTrue(np.array_equal(s["a"], [3, 4, 5, 6]))

    def test_data_outlives_the_stream(self):
        # The arrays alias Arrow's buffers, so the batch must stay alive for as long as they
        # do -- this fails with a use-after-free if ownership is not held.
        s = next(dx.stream_arrow(pa.table({"a": pa.array(range(1000), pa.int64())})))
        a = s["a"]
        del s
        import gc

        gc.collect()
        self.assertEqual(a.sum(), sum(range(1000)))

    def test_refuses_nulls(self):
        t = pa.table({"a": pa.array([1, None, 3], pa.int64())})
        with self.assertRaises(RuntimeError):
            next(dx.stream_arrow(t))

    def test_refuses_bool_rather_than_reinterpreting(self):
        # Arrow's boolean is a bitmap, one bit per value: adopting the buffer would read 8
        # values as 1. Refusing is the only correct answer without a copy.
        t = pa.table({"a": pa.array([True, False, True], pa.bool_())})
        with self.assertRaises(RuntimeError):
            next(dx.stream_arrow(t))

    def test_refuses_strings(self):
        t = pa.table({"a": pa.array(["x", "y"], pa.string())})
        with self.assertRaises(RuntimeError):
            next(dx.stream_arrow(t))

    def test_reset_rereads_a_table(self):
        # A Table can be exported again, so every epoch sees the same data.
        st = dx.stream_arrow(pa.table({"a": pa.array([1, 2], pa.int64())}))
        for _ in range(3):
            batches = list(st)
            self.assertEqual(len(batches), 1)
            self.assertTrue(np.array_equal(batches[0]["a"], [1, 2]))
            st.reset()

    def test_reset_of_a_one_shot_reader_raises(self):
        # A RecordBatchReader is consumed by the first pass; exporting it again
        # yields an empty stream. That must be an error, not a silent empty epoch.
        t = pa.table({"a": pa.array([1, 2], pa.int64())})
        reader = pa.RecordBatchReader.from_batches(t.schema, t.to_batches())
        st = dx.stream_arrow(reader)
        self.assertEqual(len(list(st)), 1)
        st.reset()
        with self.assertRaises(RuntimeError):
            next(st)

    def test_reset_of_an_empty_source_is_not_an_error(self):
        # An empty source is empty every time; there is no earlier pass to
        # compare against, so nothing is wrong.
        t = pa.table({"a": pa.array([], pa.int64())})
        reader = pa.RecordBatchReader.from_batches(t.schema, [])
        st = dx.stream_arrow(reader)
        self.assertEqual(len(list(st)), 0)
        st.reset()
        self.assertEqual(len(list(st)), 0)

    def test_rejects_a_non_arrow_object(self):
        with self.assertRaises(ValueError):
            dx.stream_arrow({"a": [1, 2, 3]})

    def test_composes_with_stream_ops(self):
        t = pa.table({"a": pa.array(range(10), pa.int64())})
        st = dx.stream_arrow(t.to_reader(max_chunksize=5)).key_transform(
            "a", lambda x: x * 2
        )
        self.assertTrue(np.array_equal(next(st)["a"], [0, 2, 4, 6, 8]))


if __name__ == "__main__":
    unittest.main()
