import os
import glob
import pandas as pd

def process_stp_files(folder_path, output_csv='instances_characteristics.csv'):
    """
    Varre um diretório em busca de arquivos .stp, extrai as informações 
    topológicas e calcula as métricas D_G e C_T.
    """
    data = []
    
    # Busca recursiva por todos os arquivos .stp na pasta e subpastas
    stp_files = glob.glob(os.path.join(folder_path, '**', '*.stp'), recursive=True)
    
    if not stp_files:
        print("Nenhum arquivo .stp encontrado no diretório especificado.")
        return

    for filepath in stp_files:
        filename = os.path.basename(filepath)
        nodes = 0
        edges = 0
        terminals = 0
        
        # Leitura do arquivo para extrair as variáveis primárias
        with open(filepath, 'r') as file:
            for line in file:
                parts = line.strip().split()
                if not parts:
                    continue
                
                # Formato SteinLib: "Nodes <num>", "Edges <num>", "Terminals <num>"
                if parts[0] == 'Nodes':
                    nodes = int(parts[1])
                elif parts[0] == 'Edges':
                    edges = int(parts[1])
                elif parts[0] == 'Terminals':
                    terminals = int(parts[1])
        
        # Cálculo das métricas topológicas propostas no artigo
        # Densidade do Grafo (D_G)
        if nodes > 1:
            density = (2 * edges) / (nodes * (nodes - 1))
        else:
            density = 0.0
            
        # Cobertura de Terminais (C_T)
        if nodes > 0:
            coverage = terminals / nodes
        else:
            coverage = 0.0
            
        # Armazena os dados processados da instância
        data.append({
            'Instância': filename,
            '|V|': nodes,
            '|E|': edges,
            '|T|': terminals,
            'D_G': round(density, 6),
            'C_T': round(coverage, 6)
        })

    # Cria um DataFrame e exporta para CSV
    df = pd.DataFrame(data)
    df.to_csv(output_csv, index=False, sep=';', decimal=',')
    
    print(f"Extração concluída com sucesso!")
    print(f"Total de instâncias processadas: {len(df)}")
    print(f"Dados salvos em: {output_csv}")

# ==========================================
# Exemplo de Execução
# ==========================================
# Especifique o caminho da pasta onde estão os seus arquivos .stp
caminho_dos_arquivos = '../../data/' # Ponto significa a pasta atual, altere se necessário

# Executa a função
process_stp_files(caminho_dos_arquivos)
