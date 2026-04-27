import csv, random
print("Generating 10M rows for Axiom Benchmark...")
with open("massive_data.csv", "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["id", "sensor_value"])
    for i in range(10_000_000):
        writer.writerow([i, random.uniform(0, 100)])
print("massive_data.csv created.")
