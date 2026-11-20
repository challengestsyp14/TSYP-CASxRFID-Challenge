"""Rule-based baseline (Card B3).

Cuts the Edge Impulse test samples into 2 s windows and classifies each window
with fixed thresholds, so the result can be compared with the trained model.

Usage:
    python ml/baseline.py                 # test samples only, export in ml/ei_export
    python ml/baseline.py path/to/export  # another export folder
    python ml/baseline.py --all           # also report on train+test
    python ml/baseline.py --min-freefall 20   # free fall must last 20 samples (200 ms)
"""

import argparse
import json
from pathlib import Path

import numpy as np

LABELS = ["carried", "drop", "impact", "upside_down"]

WINDOW = 200  # 2 s at 100 Hz
STEP = 50  # 0.5 s
G = 9.81

FREE_FALL = 4.9  # |a| below this is free fall (m/s²)
FREE_FALL_MIN_SAMPLES = 8  # default, see --min-freefall
SPIKE = 30.0  # |a| above this is a hit (m/s²)
UPSIDE_DOWN_AZ = -7.0  # mean az below this is upside down (m/s²)


def load_samples(folder, categories):
    """Return [(label, values)] for the files of the given categories, values in rows of ax..gz."""
    info = json.loads((folder / "info.labels").read_text(encoding="utf-8"))
    samples = []
    for f in info["files"]:
        if f["category"] not in categories:
            continue
        payload = json.loads((folder / f["path"]).read_text(encoding="utf-8"))["payload"]
        names = [s["name"] for s in payload["sensors"]]
        if names[:3] != ["ax", "ay", "az"]:
            raise ValueError(f"{f['path']}: unexpected sensor order {names}")
        if payload["interval_ms"] != 10:
            raise ValueError(f"{f['path']}: interval_ms is {payload['interval_ms']}, expected 10")
        samples.append((f["label"]["label"], np.asarray(payload["values"], dtype=float)))
    return samples


def detect_scale(samples):
    """Return (factor, unit): data in g needs multiplying by 9.81 to get m/s²."""
    acc = np.concatenate([values[:, :3] for _, values in samples])
    median = float(np.median(np.linalg.norm(acc, axis=1)))
    if median < 3:
        return G, f"g (median |a| = {median:.2f}), converted to m/s2 by x {G}"
    return 1.0, f"m/s2 (median |a| = {median:.2f})"


def windows(values):
    for start in range(0, len(values) - WINDOW + 1, STEP):
        yield values[start:start + WINDOW]


def classify(window, min_freefall=FREE_FALL_MIN_SAMPLES):
    acc = window[:, :3]
    mag = np.linalg.norm(acc, axis=1)
    spikes = np.flatnonzero(mag > SPIKE)

    if spikes.size:
        # end index of the first run of >= min_freefall consecutive free-fall samples
        run = 0
        for i, low in enumerate(mag < FREE_FALL):
            run = run + 1 if low else 0
            if run >= min_freefall:
                if (spikes > i).any():
                    return "drop"
                break
        return "impact"

    if acc[:, 2].mean() < UPSIDE_DOWN_AZ:
        return "upside_down"
    return "carried"


def evaluate(samples, scale, title, min_freefall=FREE_FALL_MIN_SAMPLES):
    confusion = np.zeros((len(LABELS), len(LABELS)), dtype=int)
    for label, values in samples:
        for window in windows(values * np.array([scale] * 3 + [1] * 3)):
            confusion[LABELS.index(label), LABELS.index(classify(window, min_freefall))] += 1

    totals = confusion.sum(axis=1)
    width = max(len(l) for l in LABELS) + 2

    print(f"\n=== {title}: {len(samples)} samples, {totals.sum()} windows ===\n")
    print(f"{'class':<{width}}{'windows':>9}{'correct':>9}{'accuracy':>10}")
    for i, label in enumerate(LABELS):
        acc = f"{confusion[i, i] / totals[i]:.1%}" if totals[i] else "n/a"
        print(f"{label:<{width}}{totals[i]:>9}{confusion[i, i]:>9}{acc:>10}")
    print(f"{'overall':<{width}}{totals.sum():>9}{np.trace(confusion):>9}"
          f"{np.trace(confusion) / totals.sum():>10.1%}")

    print("\nConfusion matrix (rows = true, columns = predicted)\n")
    print(" " * width + "".join(f"{l:>{width}}" for l in LABELS))
    for i, label in enumerate(LABELS):
        print(f"{label:<{width}}" + "".join(f"{n:>{width}}" for n in confusion[i]))


def main():
    parser = argparse.ArgumentParser(description="Rule-based baseline on the Edge Impulse export.")
    parser.add_argument("folder", nargs="?", default=Path(__file__).parent / "ei_export", type=Path,
                        help="unzipped Edge Impulse export (default: ml/ei_export)")
    parser.add_argument("--all", action="store_true", help="also report on train+test")
    parser.add_argument("--min-freefall", type=int, default=FREE_FALL_MIN_SAMPLES, metavar="N",
                        help=f"consecutive free-fall samples needed for a drop (default: {FREE_FALL_MIN_SAMPLES})")
    args = parser.parse_args()

    test = load_samples(args.folder, {"testing"})
    scale, unit = detect_scale(test)
    print(f"Unit detected: {unit}")
    print(f"Free fall: at least {args.min_freefall} samples ({args.min_freefall * 10} ms) below {FREE_FALL} m/s2")
    evaluate(test, scale, "Test set", args.min_freefall)

    if args.all:
        evaluate(load_samples(args.folder, {"training", "testing"}), scale, "Train + test", args.min_freefall)


if __name__ == "__main__":
    main()
