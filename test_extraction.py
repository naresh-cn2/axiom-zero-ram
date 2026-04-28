import numpy as np
import libaxiom
import time

# 1. PRE-ALLOCATION (The Top 1 Secret)
# We create space for 10,000,001 rows. 
# dtype=float64 ensures it matches the 'double' type in our C code.
price_buffer = np.zeros(10_000_001, dtype=np.float64)

target = "../massive_data.csv"

print(f"--- AXIOM EXTRACTION ENGINE V1.2 ---")
start = time.time()

# 2. THE STRIKE
# We hand the file path and our empty 'parking lot' (array) to C
actual_count = libaxiom.extract_prices(target, price_buffer)

end = time.time()

if actual_count > 0:
    print(f"SUCCESS: Extracted {actual_count:,} prices.")
    # Show the first 5 and last 5 to verify accuracy
    print(f"First 5 Prices: {price_buffer[:5]}")
    print(f"Last 5 Prices:  {price_buffer[actual_count-5:actual_count]}")
    print(f"Extraction Latency: {end - start:.6f} seconds")
else:
    print("ERROR: Engine failed to extract data.")
