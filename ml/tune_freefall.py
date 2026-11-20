"""Tune the free-fall duration of the drop rule (hybrid approach).

On the TRAINING samples only, tries several minimum free-fall lengths N and
prints how often each class is classified as drop by the rules in baseline.py.

Usage:
    python ml/tune_freefall.py                 # export in ml/ei_export
    python ml/tune_freefall.py path/to/export
"""

import argparse
from pathlib import Path

import numpy as np

from baseline import G, LABELS, classify, detect_scale, load_samples, windows

CANDIDATES = [8, 10, 15, 20, 25, 30, 35, 40]
SAMPLE_S = 0.01  # 100 Hz


def main():
    parser = argparse.ArgumentParser(description="Tune the free-fall length of the drop rule.")
    parser.add_argument("folder", nargs="?", default=Path(__file__).parent / "ei_export", type=Path,
                        help="unzipped Edge Impulse export (default: ml/ei_export)")
    args = parser.parse_args()

    train = load_samples(args.folder, {"training"})
    scale, unit = detect_scale(train)
    print(f"Unit detected: {unit}")

    factor = np.array([scale] * 3 + [1] * 3)
    by_label = {label: [] for label in LABELS}
    for label, values in train:
        by_label[label].extend(windows(values * factor))
    others = by_label["carried"] + by_label["upside_down"]

    print("Training windows: " + ", ".join(f"{l} {len(w)}" for l, w in by_label.items()))
    print()
    print(f"{'N':>4}{'min fall':>10}{'min h':>9}{'drop -> drop':>15}{'impact -> drop':>17}"
          f"{'carried/upside_down -> drop':>30}")

    for n in CANDIDATES:
        def drop_rate(ws):
            return sum(classify(w, n) == "drop" for w in ws) / len(ws)

        t = n * SAMPLE_S
        h = 0.5 * G * t ** 2
        print(f"{n:>4}{t * 1000:>8.0f} ms{h:>7.2f} m"
              f"{drop_rate(by_label['drop']):>15.1%}{drop_rate(by_label['impact']):>17.1%}"
              f"{drop_rate(others):>30.1%}")


if __name__ == "__main__":
    main()
