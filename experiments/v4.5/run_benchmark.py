import os
import subprocess
import re
import csv
import statistics

EXECUTABLE = "./steiner_forest" 
DATA_DIR = "../../data"              
ITERATIONS = "200"
RUNS_PER_INSTANCE = 10

ALGORITHMS = {
    "GRASP": {"flag": "--GRASP", "alpha": "0.5"},
    "HUB":   {"flag": "--HUB",   "alpha": "0.6"}
}

regex_first = re.compile(r"First Solution Cost:\s*([0-9.]+)")
regex_final = re.compile(r"^Solution Cost:\s*([0-9.]+)", re.MULTILINE)
regex_time  = re.compile(r"Execution Time:\s*([0-9.]+)")

def get_instances(base_dir):
    instances = []
    for root, dirs, files in os.walk(base_dir):
        for file in files:
            if file.endswith(".stp"):
                instances.append(os.path.join(root, file))
    return sorted(instances)

def run_algorithm(instance_path, flag, alpha):
    final_costs = []
    times = []
    improvements = []

    for _ in range(RUNS_PER_INSTANCE):
        command = [EXECUTABLE, "-f", instance_path, flag, "-a", alpha, "-i", ITERATIONS]
        try:
            result = subprocess.run(command, capture_output=True, text=True)
            
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
                return None
        except subprocess.TimeoutExpired:
            return None
            
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
            "imp_avg": statistics.mean(improvements)
        }
    return None

def main():
    instances = get_instances(DATA_DIR)
    csv_file = "analytical_result.csv"

    print(f"Initiating advanced A/B benchmark for {len(instances)} instances.")
    print(f"Setup: {RUNS_PER_INSTANCE} runs of {ITERATIONS} iterations per algorithm.\n")

    with open(csv_file, mode='w', newline='') as file:
        fieldnames = [
            "Instance", 
            "Winner_by_AvgCost",
            
            "GRASP_Cost_Best", "GRASP_Cost_Worst", "GRASP_Cost_Avg",
            "HUB_Cost_Best", "HUB_Cost_Worst", "HUB_Cost_Avg",
            
            "GRASP_Time_Best_ms", "GRASP_Time_Worst_ms", "GRASP_Time_Avg_ms",
            "HUB_Time_Best_ms", "HUB_Time_Worst_ms", "HUB_Time_Avg_ms",
            
            "GRASP_Improvement_Best_%", "GRASP_Improvement_Worst_%", "GRASP_Improvement_Avg_%",
            "HUB_Improvement_Best_%", "HUB_Improvement_Worst_%", "HUB_Improvement_Avg_%"
        ]
        
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        writer.writeheader()

        for instance in instances:
            filename = os.path.basename(instance)
            print(f"Processing: {filename:<20} | ", end="", flush=True)

            print("GRASP...", end="", flush=True)
            stats_g = run_algorithm(instance, ALGORITHMS["GRASP"]["flag"], ALGORITHMS["GRASP"]["alpha"])
            
            print(" HUB...", end="", flush=True)
            stats_h = run_algorithm(instance, ALGORITHMS["HUB"]["flag"], ALGORITHMS["HUB"]["alpha"])

            if not stats_g or not stats_h:
                print(" [FAIL]")
                continue

            if stats_h["cost_avg"] < stats_g["cost_avg"]:
                winner = "HUB"
            elif stats_g["cost_avg"] < stats_h["cost_avg"]:
                winner = "GRASP"
            else:
                winner = "TIE"

            row = {
                "Instance": filename,
                "Winner_by_AvgCost": winner,
                
                "GRASP_Cost_Best": round(stats_g["cost_best"], 4),
                "GRASP_Cost_Worst": round(stats_g["cost_worst"], 4),
                "GRASP_Cost_Avg": round(stats_g["cost_avg"], 4),
                
                "HUB_Cost_Best": round(stats_h["cost_best"], 4),
                "HUB_Cost_Worst": round(stats_h["cost_worst"], 4),
                "HUB_Cost_Avg": round(stats_h["cost_avg"], 4),
                
                "GRASP_Time_Best_ms": round(stats_g["time_best"], 3),
                "GRASP_Time_Worst_ms": round(stats_g["time_worst"], 3),
                "GRASP_Time_Avg_ms": round(stats_g["time_avg"], 3),
                
                "HUB_Time_Best_ms": round(stats_h["time_best"], 3),
                "HUB_Time_Worst_ms": round(stats_h["time_worst"], 3),
                "HUB_Time_Avg_ms": round(stats_h["time_avg"], 3),
                
                "GRASP_Improvement_Best_%": round(stats_g["imp_best"], 2),
                "GRASP_Improvement_Worst_%": round(stats_g["imp_worst"], 2),
                "GRASP_Improvement_Avg_%": round(stats_g["imp_avg"], 2),
                
                "HUB_Improvement_Best_%": round(stats_h["imp_best"], 2),
                "HUB_Improvement_Worst_%": round(stats_h["imp_worst"], 2),
                "HUB_Improvement_Avg_%": round(stats_h["imp_avg"], 2)
            }
            
            writer.writerow(row)
            file.flush() 
            print(f" | Winner: {winner}")

    print(f"\nExperiments completed successfully! Results saved in '{csv_file}'.")

if __name__ == "__main__":
    main()
