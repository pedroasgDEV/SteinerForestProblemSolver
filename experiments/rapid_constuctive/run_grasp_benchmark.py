import os
import subprocess
import csv

EXECUTABLE = "./steiner_forest" 
DATA_DIR = "../../../data"              
ITERATIONS = "10000"
RUNS_PER_INSTANCE = 10
CSV_FILE = "grasp_selected_instances.csv"

TARGET_ALGO = "GRASP" 
RES_DIR = "grasp_res"
FLAG = "--GRASP"
ALPHA = "0.5"

def load_time_limits(csv_path):
    time_limits = {}
    if not os.path.exists(csv_path):
        return time_limits
        
    with open(csv_path, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            time_limits[row['Instance']] = float(row['Time_Best_ms']) / 1000.0
    return time_limits

def get_instances_paths(base_dir, target_instances):
    paths = {}
    for root, dirs, files in os.walk(base_dir):
        for file in files:
            if file in target_instances:
                paths[file] = os.path.join(root, file)
    return paths

def main():
    time_limits = load_time_limits(CSV_FILE)
    if not time_limits:
        return

    instances_paths = get_instances_paths(DATA_DIR, set(time_limits.keys()))
    
    os.makedirs(RES_DIR, exist_ok=True)
    
    for instance_name, time_limit in time_limits.items():
        path = instances_paths.get(instance_name)
        if not path:
            print(f"[{instance_name}] Arquivo .stp não encontrado em {DATA_DIR}.")
            continue
            
        out_filepath = os.path.join(RES_DIR, f"{instance_name}.out")
        command = [EXECUTABLE, "-f", path, FLAG, "-a", ALPHA, "-i", ITERATIONS]
        
        with open(out_filepath, "w", encoding="utf-8") as f_out:
            for run_idx in range(RUNS_PER_INSTANCE):
                f_out.write(f"--- RUN {run_idx + 1}/{RUNS_PER_INSTANCE} ---\n")
                
                try:
                    result = subprocess.run(command, capture_output=True, text=True, timeout=time_limit)
                    f_out.write(result.stdout)
                except subprocess.TimeoutExpired as e:
                    output_str = e.stdout.decode('utf-8', errors='ignore') if isinstance(e.stdout, bytes) else (e.stdout or "")
                    f_out.write(output_str)
                
                f_out.write("\n")
                
if __name__ == "__main__":
    main()
