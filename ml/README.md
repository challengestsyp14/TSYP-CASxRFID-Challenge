# ML model (Edge Impulse)

**The model the tag actually runs is library 1.0.6, in `tag/`. Its test results are in `tag/README.md`.**
Everything below is the development history of earlier versions; those numbers describe those
versions, not the deployed one. The model exports for them are not kept in this repository.

## v3 – 4 classes with still

- Labels: carried, impact, still, upside_down. Drop is still detected in the firmware by the free-fall rule (N = 15, unchanged); `still` produces no event, like carried.
- Data: rebuilt dataset, 220 samples, with the train/test split set by hand. Samples per class (train / test): carried 30 / 7, impact 109 / 34, still 17 / 3, upside_down 15 / 5.
- Impulse: same as v2 (2000 ms window, 500 ms increase, 100 Hz, 6 axes, Spectral Analysis + neural network, auto-weight classes, no rebalance).
- Test results, quantized int8 model (the one deployed): overall accuracy 97.49%, ROC AUC 1.00, weighted precision / recall / F1 0.98.

| Class | Accuracy (int8) | F1 |
|---|---|---|
| carried | 96.6% | 0.98 |
| impact | 96.2% | 0.98 |
| still | 100% | 1.00 |
| upside_down | 96.7% | 0.95 |

- Flags: no class is below 90%. carried ↔ impact: 0% of carried predicted as impact, 1.0% of impact as carried. carried ↔ upside_down: 2.0% of carried predicted as upside_down, 1.6% of upside_down as carried. The rest goes to uncertain (carried 1.4%, impact 1.9%, upside_down 1.6%) or impact → upside_down (1.0%).
- The test set is small (still 3, upside_down 5 samples), so the per-class percentages for those classes are not reliable.
- On-device (Edge Impulse estimate, ESP-EYE ESP32 240 MHz, EON):

| | Spectral features | Classifier | Total |
|---|---|---|---|
| int8 latency | 86 ms | 1 ms | 87 ms |
| int8 RAM | 5.6 KB | 1.4 KB | 5.6 KB |
| int8 flash | – | 15.9 KB | – |
| float32 latency | 86 ms | 3 ms | 89 ms |
| float32 RAM | 5.6 KB | 1.6 KB | 5.6 KB |
| float32 flash | – | 17.7 KB | – |

- float32 test accuracy is 96.81% vs 97.49% for int8, so int8 was kept (less flash, faster classifier, same total RAM).
- Latency is the same estimate as v2 (87 ms total), since the spectral features step dominates and is unchanged. The estimate is for 240 MHz, while the tag runs at 160 MHz; the old model measured 51–57 ms on the tag, and the target is about 60 ms or less. The real time must be measured on the tag.
- Deployment: Arduino library, quantized int8, EON Compiler (library 1.0.5; superseded by 1.0.6 and not kept here).
- Figures: docs/confusion_matrix_v3.png, docs/accuracy_v3.png, docs/deployment_v3.png.
- Old data: ml/ei_export, `ml/baseline.py` and `ml/tune_freefall.py` use the OLD dataset. The new dataset has no drop label, so the free-fall threshold (N = 15) was not re-tuned.

## v1 – 4 classes with drop (history)

- Tool: Edge Impulse. Impulse: time series, 2000 ms window, 500 ms increase, 100 Hz, Spectral Analysis + Classification (neural network, 30 training cycles, learning rate 0.0005, auto-weight classes).
- Data: 107 training samples and 31 test samples (78/22 split). Training data per class: carried ≈240 s, drop ≈115 s, impact ≈145 s, upside_down ≈40 s.
  Samples per class (train / test): carried 12 / 3, drop 38 / 9, impact 48 / 17, upside_down 9 / 2 (total 107 / 31).
- Test set: Edge Impulse's 78/22 split of our collection; a separate session will be recorded as a held-out test set.
- Test results, quantized int8 model (the one deployed): overall accuracy 78.61%, ROC AUC 0.96, weighted F1 0.82. Per class: carried 93.7% (F1 0.97), drop 66.7% (F1 0.55), impact 51.0% (F1 0.61), upside_down 83.3% (F1 0.77).
- Main confusion: drop vs impact (22% of drops predicted as impact, 39% of impacts as drop). Possible fix: merge them into one "mishandling" class, since the firmware gets drop height from free fall anyway.
- Edge Impulse estimate on ESP32 240 MHz (int8, EON): latency 87 ms (86 ms spectral features + 1 ms classifier), RAM 5.6 KB total including the spectral features step (from the Deployment page table), classifier flash 15.9 KB (no estimate given for the spectral features). Real inference time on the tag still to be measured.
- float32 gives the same accuracy (78.61%) but 89 ms and 17.7 KB classifier flash, so int8 was chosen.
- Figures: docs/confusion_matrix_test.png and docs/accuracy_per_class.png
- Deployment: Arduino library, quantized int8, EON Compiler. The v1 zip was replaced by the v2 zip (library 1.0.4, not kept here); the v1 model is still saved in Edge Impulse as version "v1 4 classes int8".

## Baseline vs model

Rule-based baseline (`ml/baseline.py`) on the same 31 test samples, cut into 2 s windows with a 0.5 s step.

| Class | Rules (baseline) | Edge Impulse model (int8) |
|---|---|---|
| carried | 100% | 93.7% |
| drop | 100% | 66.7% |
| impact | 58.8% | 51.0% |
| upside_down | 100% | 83.3% |
| Overall (windows) | 89.6% | 78.6% |
| Mean of 4 classes | 89.7% | 73.7% |

1. Both confuse impact with drop: 21/51 impact windows contain ≥8 samples below 4.9 m/s² before the spike, so the impact recordings likely include a short free fall.
2. The free-fall rule is a strong physical signal for drops, while the model has little drop data.
3. Not a strictly equal comparison: window cutting differs, Edge Impulse counts "uncertain" as wrong, and carried dominates the window counts.
4. Thresholds come from the task card and were not tuned on this data.

Setup (once, from the repo root):
1. Create the venv: `python -m venv ml\venv`
2. Install numpy: `ml\venv\Scripts\python.exe -m pip install numpy`
3. Export the data from Edge Impulse (Data acquisition → Export data) and unzip it into `ml\ei_export\` (git-ignored), keeping `info.labels`.

Run: `ml\venv\Scripts\python.exe ml\baseline.py` (add `--all` for train+test).

## v2 hybrid

- Approach: drop = free-fall rule in the firmware (|a| < 4.9 m/s² for at least 15 samples = 150 ms, about 11 cm of fall, followed by a spike > 30 m/s²); the model classifies carried / impact / upside_down. Drop samples were relabeled as impact for training (a drop ends with an impact spike).
- Free-fall tuning on training data only (`ml/tune_freefall.py`): N = 15 keeps 91.2% of drops while impacts wrongly called drop fall from 42.4% (N = 10) to 2.1%.

| N | Min fall | Min height | Drop → drop | Impact → drop | Carried/upside_down → drop |
|---|---|---|---|---|---|
| 8 | 80 ms | 0.03 m | 99.1% | 52.1% | 0.0% |
| 10 | 100 ms | 0.05 m | 99.1% | 42.4% | 0.0% |
| **15** | **150 ms** | **0.11 m** | **91.2%** | **2.1%** | **0.0%** |
| 20 | 200 ms | 0.20 m | 57.9% | 2.1% | 0.0% |
| 25 | 250 ms | 0.31 m | 44.7% | 2.1% | 0.0% |
| 30 | 300 ms | 0.44 m | 31.6% | 1.4% | 0.0% |
| 35 | 350 ms | 0.60 m | 18.4% | 1.4% | 0.0% |
| 40 | 400 ms | 0.78 m | 7.9% | 1.4% | 0.0% |

- Rule on the test set with N = 15 (`ml/baseline.py --min-freefall 15`): 95.5% overall; carried 100%, drop 88.9%, impact 88.2%, upside_down 100%.
- 3-class model, int8 test results: overall accuracy 91.04%; carried 91.9%, impact 89.7%, upside_down 91.7%; F1 0.95 / 0.95 / 0.71 (upside_down has only 2 test samples). Impact improved from 51.0% (v1) to 89.7%.
- Validation accuracy during training was 96.7% (carried 94.2%, impact 100%, upside_down 100%), but the test numbers are the ones we report.
- On-device (Edge Impulse, ESP32 240 MHz, int8, EON): classifier 1 ms, 1.4 KB peak RAM for the classifier alone (5.6 KB total including the spectral features step, from the Deployment page table), 15.8 KB flash; the spectral features step adds about 86 ms.
- float32 gives 93.03% test accuracy vs 91.04% for int8; int8 was kept because it uses less flash (15.8 KB vs 17.7 KB) and the classifier runs faster (1 ms vs 3 ms), with the same 5.6 KB total RAM (including the spectral features step).
- Figures: docs/confusion_matrix_v2.png, docs/accuracy_v2.png. Edge Impulse version: "v2 hybrid 3 classes int8".
