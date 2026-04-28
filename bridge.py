import ctypes
import os
import time

# 1. Load the Engine
lib_path = os.path.abspath("./libaxiom.so")
axiom = ctypes.CDLL(lib_path)

# 2. Define the Handshake
axiom.run_axiom_parser.argtypes = [ctypes.c_char_p]
axiom.run_axiom_parser.restype = ctypes.c_longlong

def count_with_axiom(filepath):
    path_bytes = filepath.encode('utf-8')
    
    print(f"--- AXIOM SYSTEM V1.1 INITIATED ---")
    start = time.time()
    
    # Call the C function from Python
    result = axiom.run_axiom_parser(path_bytes)
    
    end = time.time()
    
    if result == -1:
        print("Error: Could not open file.")
    else:
        print(f"Success! Found {result:,} rows.")
        print(f"Engine Time: {end - start:.6f} seconds")

# 3. Test on your data
# Note: make sure the path to your CSV is correct
target = "../massive_data.csv"
count_with_axiom(target)
