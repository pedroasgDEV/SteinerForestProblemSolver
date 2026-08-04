import os
import subprocess
import re
import csv
import statistics
import time

EXECUTABLE = "./steiner_forest" 
DATA_DIR = "../../data"              
ITERATIONS = "200"
RUNS_PER_INSTANCE = 10
TIME_LIMIT_SECONDS = 3600  

TARGET_ALGO = "HUB" 
LOG_FILE = "hub.log" 
CSV_FILE = f"{TARGET_ALGO.lower()}_analytical_result_resumed.csv"

ALGORITHMS = {
    "GRASP": {"flag": "--GRASP", "alpha": "0.5"},
    "HUB":   {"flag": "--HUB",   "alpha": "0.6"}
}

regex_first = re.compile(r"First Solution Cost:\s*([0-9.]+)")
regex_final = re.compile(r"^Solution Cost:\s*([0-9.]+)", re.MULTILINE)
regex_time  = re.compile(r"Execution Time:\s*([0-9.]+)")

def get_processed_instances(log_path):
    processed = set()
    if os.path.exists(log_path):
        with open(log_path, "r", encoding="utf-8", errors="ignore") as f:
            content = f.read()
            matches = re.findall(r"Processing:\s*(\S+\.stp)", content)
            processed.update(matches)
    return processed

def get_instances(base_dir, processed_set):
    instances = []
    for root, dirs, files in os.walk(base_dir):
        for file in files:
            if file.endswith(".stp") and file not in processed_set:
                instances.append(os.path.join(root, file))
    return sorted(instances)

def run_algorithm(instance_path, flag, alpha):
    final_costs = []
    times = []
    improvements = []
    
    start_time = time.time()

    for run_idx in range(RUNS_PER_INSTANCE):
        elapsed_time = time.time() - start_time
        remaining_time = TIME_LIMIT_SECONDS - elapsed_time
        
        if remaining_time <= 0:
            break 
            
        command = [EXECUTABLE, "-f", instance_path, flag, "-a", alpha, "-i", ITERATIONS]
        
        try:
            result = subprocess.run(command, capture_output=True, text=True, timeout=remaining_time)
            
            first_match = regex_first.search(result.stdout)
            final_match = regex_final.search(result.stdout)
            time_match  = regex_time.search(result.stdout)
            
            if first_match and final_match and time_match:
                first_c = float(first_match.group(1))
                final_c = float(final_match.group(1))
                time_ms = float(time_match.group(1))
                
                if first_c > 0:
                    imp_percent = ((first_c - final_c) / first_c) * 100.0
                else:
                    imp_percent = 0.0

                final_costs.append(final_c)
                times.append(time_ms)
                improvements.append(imp_percent)
            else:
                continue
                
        except subprocess.TimeoutExpired:
            break 
            
    if final_costs and times:
        return {
            "cost_best": min(final_costs),
            "cost_worst": max(final_costs),
            "cost_avg": statistics.mean(final_costs),
            
            "time_best": min(times),
            "time_worst": max(times),
            "time_avg": statistics.mean(times),
            
            "imp_best": max(improvements), 
            "imp_worst": min(improvements), 
            "imp_avg": statistics.mean(improvements),
            
            "runs_completed": len(final_costs)
        }
    
    return None

def main():
    processed = get_processed_instances(LOG_FILE)
    print(f"Encontradas {len(processed)} instâncias já processadas no arquivo '{LOG_FILE}'. Elas serão puladas.")
    
    instances = get_instances(DATA_DIR, processed)
    
    print(f"Iniciando benchmark avançado para as {len(instances)} instâncias restantes.")
    print(f"Setup: Máximo de {RUNS_PER_INSTANCE} execuções de {ITERATIONS} iterações. Limite: 1h por instância.\n")

    file_exists = os.path.isfile(CSV_FILE)

    with open(CSV_FILE, mode='a', newline='') as file: 
        fieldnames = [
            "Instance", 
            
            "Cost_Best", "Cost_Worst", "Cost_Avg",
            
            "Time_Best_ms", "Time_Worst_ms", "Time_Avg_ms",
            
            "Improvement_Best_%", "Improvement_Worst_%", "Improvement_Avg_%",
            
            "Runs_Completed" 
        ]
        
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        if not file_exists:
            writer.writeheader()

        for instance in instances:
            filename = os.path.basename(instance)
            print(f"Processing: {filename:<20}...", end="", flush=True)
            
            stats = run_algorithm(instance, ALGORITHMS[TARGET_ALGO]["flag"], ALGORITHMS[TARGET_ALGO]["alpha"])

            if not stats:
                print(" [FALHOU OU TIMEOUT TOTAL]")
                continue

            row = {
                "Instance": filename,
                
                "Cost_Best": round(stats["cost_best"], 4),
                "Cost_Worst": round(stats["cost_worst"], 4),
                "Cost_Avg": round(stats["cost_avg"], 4),
                
                "Time_Best_ms": round(stats["time_best"], 3),
                "Time_Worst_ms": round(stats["time_worst"], 3),
                "Time_Avg_ms": round(stats["time_avg"], 3),
                
                "Improvement_Best_%": round(stats["imp_best"], 2),
                "Improvement_Worst_%": round(stats["imp_worst"], 2),
                "Improvement_Avg_%": round(stats["imp_avg"], 2),
                
                "Runs_Completed": stats["runs_completed"]
            }
            
            writer.writerow(row)
            file.flush()
            print(f" [OK] ({stats['runs_completed']}/{RUNS_PER_INSTANCE} execuções)")

    print(f"\nExperimentos concluídos! Resultados salvos em '{CSV_FILE}'.")

if __name__ == "__main__":
    main()
