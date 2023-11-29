import time
from collections import defaultdict


class Benchmark:
    def __init__(self, name):
        self.name = name
        self.runtimes = defaultdict(list)
        self.n_samples = defaultdict(int)

    def log_run(self, run_name, fn, *args, **kwargs):
        start = time.time()
        n_samples = fn(*args, **kwargs)
        end = time.time()
        self.runtimes[run_name].append(end - start)
        self.n_samples[run_name] = n_samples

    def report(self):
        print(f"Benchmark {self.name}")
        print()
        klengths = max(map(len, self.runtimes.keys()))
        table = []
        for k in self.runtimes.keys():
            n_runs = len(self.runtimes[k])
            avg_time = sum(self.runtimes[k]) / n_runs
            avg_throughput = self.n_samples[k] / avg_time
            table.append(
                [
                    k,
                    f"{avg_time:.3f} s",
                    f"{avg_throughput:.3f} samples/s",
                    f"{n_runs} runs",
                ]
            )

        column_widths = [
            max(len(table[j][i]) for j in range(len(table)))
            for i in range(len(table[0]))
        ]
        for row in table:
            for i, (v, w) in enumerate(zip(row, column_widths)):
                if i == 0:
                    print(v, " " * (w - len(v)), " " * 4, end="")
                else:
                    print(" " * (w - len(v)), v, " " * 4, end="")
            print()
