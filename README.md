# 🚀 Axiom Core: The Hydra Analyst
**Extreme-Performance Native Ingestion & Signal Engine**

Standard data pipelines (Python/Pandas) pay a massive "Abstraction Tax." Axiom Core bypasses the bloat by talking directly to hardware. Built in C with a multi-threaded architecture, it treats your SSD as a direct extension of RAM.

### ⚡ The 10 Million Row Benchmark
* **Dataset:** 10,000,000 Rows (CSV)
* **Axiom C-Engine:** **0.26 seconds** * **Performance Gain:** ~10x faster than standard Python methods.

### 🧠 Core Architecture
* **The Hydra:** Multi-threaded ingestion using `pthreads` to bypass the Python GIL.
* **Zero-Copy:** Maps files directly into memory addresses using `mmap`.
* **The Analyst:** Hardware-level filtering (Threshold + Volatility) during ingestion.

### 🛠️ Usage
1. **Install:** `pip install .`
2. **Execute:**
   ```python
   import libaxiom
   import numpy as np
   buffer = np.zeros(10_000_001, dtype=np.float64)
   signals = libaxiom.extract_prices("massive_data.csv", buffer, 50000.0, 10.0)
