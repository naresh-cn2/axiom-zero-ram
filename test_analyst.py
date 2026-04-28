import numpy as np
import libaxiom
import time

# Create a buffer for the signals
# We use the same size, but we expect only a small fraction to pass the filter
signal_buffer = np.zeros(10_000_001, dtype=np.float64)

# COMMANDER'S SETTINGS
THRESHOLD = 1000.0  # Only look at prices above this
VOLATILITY = 0.5    # Only look at sudden changes larger than this

print(f"--- AXIOM ANALYST V1.5: HUNTING BREAKOUTS ---")
start = time.time()

# The Strike: C will now filter the 10M rows while it reads them
count = libaxiom.extract_prices("../massive_data.csv", signal_buffer, THRESHOLD, VOLATILITY)

end = time.time()

print(f"Signals Found: {count:,}")
if count > 0:
    print(f"First 5 Signals Identified: {signal_buffer[:5]}")
    print(f"Analysis Latency: {end - start:.6f} seconds")
else:
    print("No market anomalies found with these settings.")
