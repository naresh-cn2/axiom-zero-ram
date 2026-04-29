import ctypes
import os
import numpy as np

# Load the shared library
lib_path = os.path.join(os.path.dirname(__file__), 'libaxiom.so')
axiom_lib = ctypes.CDLL(lib_path)

# Correct the C-function signature (4 arguments)
# long long run_analyst_engine(const char *path, double *out, double T, double V)
axiom_lib.run_analyst_engine.argtypes = [
    ctypes.c_char_p,                 # path
    ctypes.POINTER(ctypes.c_double), # out_buffer
    ctypes.c_double,                 # T (Threshold)
    ctypes.c_double                  # V (Volatility)
]
axiom_lib.run_analyst_engine.restype = ctypes.c_longlong

def run_axiom_engine(filepath):
    if not os.path.exists(filepath):
        return 0, 0
    
    # 1. Allocate a results buffer (Max 1M signals per benchmark)
    # This prevents the "out[found]" crash at Line 60
    max_results = 10_000_001
    results_buffer = np.zeros(max_results, dtype=np.float64)
    out_ptr = results_buffer.ctypes.data_as(ctypes.POINTER(ctypes.c_double))
    
    # 2. Parameters
    c_filepath = filepath.encode('utf-8')
    threshold = 0.5
    volatility = 0.01
    
    # 3. Trigger the Hydra
    total_signals = axiom_lib.run_analyst_engine(c_filepath, out_ptr, threshold, volatility)
    
    file_size_gb = os.path.getsize(filepath) / (1024**3)
    return total_signals, file_size_gb
