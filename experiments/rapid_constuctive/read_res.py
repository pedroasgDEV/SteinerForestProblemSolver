import os
import json
import re

def parse_log_file(file_path):
    regex_run = re.compile(r"^---\s*RUN\s*\d+/\d+\s*---")
    regex_data = re.compile(r"^\s*(\d+)\s+([0-9.]+)\s+([0-9.]+)")
    runs_list = []
    current_run = None
    
    if not os.path.exists(file_path):
        return runs_list

    with open(file_path, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
                
            if regex_run.match(line):
                if current_run is not None:
                    runs_list.append(current_run)
                current_run = []
            else:
                match = regex_data.match(line)
                if match and current_run is not None:
                    it_id = int(match.group(1))
                    exec_time = float(match.group(2))
                    cost_val = int(float(match.group(3)))
                    
                    current_run.append({
                        "id": it_id,
                        "time": exec_time,
                        "value": cost_val
                    })
                    
    if current_run is not None:
        runs_list.append(current_run)
        
    return runs_list

def generate_combined_json(grasp_dir, hub_dir, output_file):
    results = {}
    
    grasp_files = set(f for f in os.listdir(grasp_dir) if f.endswith(".out")) if os.path.exists(grasp_dir) else set()
    hub_files = set(f for f in os.listdir(hub_dir) if f.endswith(".out")) if os.path.exists(hub_dir) else set()
    
    all_files = grasp_files.union(hub_files)
    
    for filename in all_files:
        instance_name = filename[:-4]
        
        grasp_path = os.path.join(grasp_dir, filename)
        hub_path = os.path.join(hub_dir, filename)
        
        grasp_runs = parse_log_file(grasp_path)
        hub_runs = parse_log_file(hub_path)
        
        results[instance_name] = {
            "grasp": grasp_runs,
            "hub": hub_runs
        }
        
    with open(output_file, 'w', encoding='utf-8') as f_json:
        json.dump(results, f_json, indent=4)

if __name__ == "__main__":
    generate_combined_json("grasp_res", "hub_res", "combined_data.json")
