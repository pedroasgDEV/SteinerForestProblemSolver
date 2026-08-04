import os
import glob
import pandas as pd

def process_stp_files(folder_path, output_csv='instances_characteristics.csv'):
    data = []
    
    stp_files = glob.glob(os.path.join(folder_path, '**', '*.stp'), recursive=True)
    
    for filepath in stp_files:
        filename = os.path.basename(filepath)
        nodes = 0
        edges = 0
        terminals = 0
        
        with open(filepath, 'r') as file:
            for line in file:
                parts = line.strip().split()
                if not parts:
                    continue
                
                if parts[0] == 'Nodes':
                    nodes = int(parts[1])
                elif parts[0] == 'Edges':
                    edges = int(parts[1])
                elif parts[0] == 'Terminals':
                    terminals = int(parts[1])
        
        if nodes > 1:
            density = (2 * edges) / (nodes * (nodes - 1))
        else:
            density = 0.0
            
        if nodes > 0:
            coverage = terminals / nodes
        else:
            coverage = 0.0
            
        data.append({
            'Instância': filename,
            '|V|': nodes,
            '|E|': edges,
            '|T|': terminals,
            'D_G': round(density, 6),
            'C_T': round(coverage, 6)
        })

    df = pd.DataFrame(data)
    df.to_csv(output_csv, index=False, sep=';', decimal=',')
    
dir_path = '../../data/' 

process_stp_files(dir_path)
