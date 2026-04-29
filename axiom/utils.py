import time
import pandas as pd
import platform
import subprocess

def run_pandas_baseline(file_path):
    """Measures standard Pandas ingestion speed."""
    start = time.perf_counter()
    try:
        df = pd.read_csv(file_path)
        end = time.perf_counter()
        return (end - start), "Success"
    except Exception as e:
        return 0, f"Failed ({str(e)})"

def get_hardware_info():
    """Detects CPU and Disk type for credibility."""
    info = {"cpu": platform.processor(), "disk_type": "SSD/NVMe"}
    try:
        # Simple check for WSL users
        cpu_name = subprocess.check_output("grep -m 1 'model name' /proc/cpuinfo", shell=True).decode().split(":")[1].strip()
        info["cpu"] = cpu_name
    except:
        pass
    return info
