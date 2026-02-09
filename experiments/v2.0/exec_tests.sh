#!/bin/bash

# ================= CONFIGURAÇÕES =================
# Caminho para o executável
SOLVER="./steiner_forest"

# Caminho base para os dados (conforme você especificou)
DATA_ROOT="../../data/"

# Pasta onde os relatórios .md serão salvos
OUTPUT_DIR="."

# Lista de subdiretórios para processar
# Você pode adicionar ou remover linhas aqui conforme necessário
DIRS=(
    "Sparse-Graphs/Incidence/I080"
    "Sparse-Graphs/Incidence/I160"
    "Sparse-Graphs/Incidence/I320"
    "Sparse-Graphs/Incidence/I640"
    "Sparse-Graphs/Random"
    "Sparse-Graphs"
    # "WireRouting-Graphs"
)
# =================================================

# Verifica se o solver existe
if [ ! -f "$SOLVER" ]; then
    echo "Erro: Executável $SOLVER não encontrado."
    exit 1
fi

# Cria a pasta de saída se não existir
mkdir -p "$OUTPUT_DIR"

echo "Iniciando geração de relatórios..."
echo "---------------------------------"

# Loop através de cada diretório definido
for SUBDIR in "${DIRS[@]}"; do
    FULL_PATH="$DATA_ROOT/$SUBDIR"
    
    # Verifica se o diretório de dados existe
    if [ ! -d "$FULL_PATH" ]; then
        echo "Aviso: Diretório não encontrado: $FULL_PATH (Pulando...)"
        continue
    fi

    # Define o nome do arquivo de saída. 
    # Ex: Sparse-Graphs/Incidence/I080 -> I080.md
    # Se preferir o nome completo (Sparse-Graphs_Incidence_I080), troque basename por: echo $SUBDIR | tr '/' '_'
    if [ "$SUBDIR" == "." ]; then
        DIR_NAME="all"
    else
        DIR_NAME=$(basename "$SUBDIR")
    fi

    REPORT_FILE="$OUTPUT_DIR/${DIR_NAME}.md"

    echo "Processando: $DIR_NAME ..."

    # 1. Find best Alpha value (-v)
    echo "# 1. Find best Alpha value (-v)" >> "$REPORT_FILE"
    echo '```' >> "$REPORT_FILE"
    $SOLVER -d "$FULL_PATH" -v >> "$REPORT_FILE"
    echo '```' >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"

    # 1. Alpha 0 (-a 0)
    echo "# 2. Alpha 0 (Greedy)" >> "$REPORT_FILE"
    echo '```' >> "$REPORT_FILE"
    $SOLVER -d "$FULL_PATH" -a 0 >> "$REPORT_FILE"
    echo '```' >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"

    # 2. Alpha 1 (-a 1)
    echo "# 3. Alpha 1 (Random)" >> "$REPORT_FILE"
    echo '```' >> "$REPORT_FILE"
    $SOLVER -d "$FULL_PATH" -a 1 >> "$REPORT_FILE"
    echo '```' >> "$REPORT_FILE"

    echo "  -> Relatório salvo em: $REPORT_FILE"
done

echo "---------------------------------"
echo "Concluído! Verifique a pasta '$OUTPUT_DIR'."
