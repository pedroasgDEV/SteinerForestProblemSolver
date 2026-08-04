import os
import re

# --- CONFIGURAÇÕES DE TESTE ---
DATA_DIR = "../../data"              
LOG_FILE = "grasp.log" # Troque para "hub.log" para testar o outro log
# ------------------------------

def get_processed_instances(log_path):
    """Lê o arquivo .log e retorna um set com os nomes das instâncias já processadas."""
    processed = set()
    if os.path.exists(log_path):
        with open(log_path, "r", encoding="utf-8", errors="ignore") as f:
            content = f.read()
            # Captura o nome do arquivo que vem após "Processing: "
            matches = re.findall(r"Processing:\s*(\S+\.stp)", content)
            processed.update(matches)
    return processed

def get_instances(base_dir, processed_set):
    """Busca os .stp no diretório, ignorando os que já estão no processed_set."""
    instances = []
    
    # Adicionado um aviso amigável caso o diretório de dados não seja encontrado no teste
    if not os.path.exists(base_dir):
        print(f"\n[AVISO] O diretório '{base_dir}' não foi encontrado.")
        print("Verifique se você está rodando o script na pasta correta.\n")
        return []

    for root, dirs, files in os.walk(base_dir):
        for file in files:
            if file.endswith(".stp") and file not in processed_set:
                instances.append(os.path.join(root, file))
    return sorted(instances)

def main():
    print("=== TESTE DE FILTRO DE INSTÂNCIAS ===")
    print(f"Lendo histórico do arquivo: {LOG_FILE}")
    
    processed = get_processed_instances(LOG_FILE)
    print(f"-> Total de instâncias identificadas como CONCLUÍDAS: {len(processed)}")
    
    remaining_instances = get_instances(DATA_DIR, processed)
    print(f"-> Total de instâncias PENDENTES encontradas na pasta: {len(remaining_instances)}\n")
    
    if remaining_instances:
        print("Lista de instâncias que seriam processadas (primeiras 50 para não poluir a tela):")
        for instance in remaining_instances:
            print(f" - {os.path.basename(instance)}")
            
    else:
        print("Nenhuma instância pendente encontrada. Todas já foram processadas ou a pasta está vazia.")

if __name__ == "__main__":
    main()
