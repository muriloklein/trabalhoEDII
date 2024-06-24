#include <iostream>     // Biblioteca para entrada e saída padrão
#include <fstream>      // Biblioteca para manipulação de arquivos
#include <string>       // Biblioteca para manipulação de strings
#include <map>          // Biblioteca para uso de mapas (tabela de associação chave-valor)
#include <queue>        // Biblioteca para uso de filas de prioridade
#include <locale>       // Biblioteca para manipulação de localidades
#include <codecvt>      // Biblioteca para conversão de caracteres entre diferentes codificações
#include <cstdlib>      // Biblioteca padrão de funções úteis
#include <sys/stat.h>   // Biblioteca para funções de sistema relacionadas a informações de arquivos
#include <cerrno>       // Biblioteca para manipulação de códigos de erro

#ifdef _WIN32
#include <direct.h>     // Biblioteca para funções de sistema específicas do Windows
#endif

using namespace std;    // Utiliza o namespace std para facilitar o uso de elementos da biblioteca padrão

// Função para analisar o arquivo e calcular as frequências de cada caractere
map<wchar_t, int> analyzeFile(const wstring &filename) {
    // Converter wstring para string UTF-8
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
    // Converte o nome do arquivo de wstring para string UTF-8
    string utf8_filename = converter.to_bytes(filename);

    // Abrir o arquivo usando wifstream para suportar caracteres wide
    wifstream wfile(utf8_filename);
    // Configura o fluxo de entrada para usar a localidade atual e converter caracteres UTF-8 para wide
    wfile.imbue(locale(wfile.getloc(), new codecvt_utf8<wchar_t>));

    // Verifica se o arquivo foi aberto corretamente
    if (!wfile.is_open()) {
        wcerr << L"Erro ao abrir o arquivo " << filename << endl;
        return {};  // Retorna um mapa vazio se não conseguir abrir o arquivo
    }

    map<wchar_t, int> frequencies;  // Mapa para armazenar as frequências dos caracteres
    wchar_t ch;
    // Lê cada caractere do arquivo e atualiza suas frequências no mapa
    while (wfile.get(ch)) {
        frequencies[ch]++;
    }
    wfile.close();  // Fecha o arquivo após a leitura
    return frequencies;  // Retorna o mapa com as frequências dos caracteres
}

// Estrutura para representar um nó na árvore de Huffman
struct HuffmanNode {
    wchar_t symbol;     // Símbolo armazenado no nó
    int frequency;      // Frequência do símbolo
    HuffmanNode *left;  // Ponteiro para o nó filho esquerdo na árvore de Huffman
    HuffmanNode *right; // Ponteiro para o nó filho direito na árvore de Huffman

    // Construtor para inicializar um nó da árvore de Huffman
    HuffmanNode(wchar_t s, int f) : symbol(s), frequency(f), left(nullptr), right(nullptr) {}

    // Método para imprimir o nó em formato DOT para Graphviz
    void printNode(ofstream& dot) const {
        dot << "\t" << reinterpret_cast<uintptr_t>(this) << " [label=\"" << frequency << "\"];\n";
    }
};

// Functor para comparar dois nós na fila de prioridade
struct compare {
    // Operador de comparação para ordenar os nós na fila de prioridade
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return (l->frequency > r->frequency);
    }
};

// Função para construir a árvore de Huffman a partir das frequências dos caracteres
HuffmanNode* buildHuffmanTree(const map<wchar_t, int> &frequencies) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, compare> pq;  // Fila de prioridade para construir a árvore de Huffman
    // Insere todos os símbolos e suas frequências na fila de prioridade
    for (auto pair : frequencies) {
        pq.push(new HuffmanNode(pair.first, pair.second));
    }
    // Constrói a árvore de Huffman combinando os nós com as menores frequências
    while (pq.size() > 1) {
        HuffmanNode *left = pq.top(); pq.pop();
        HuffmanNode *right = pq.top(); pq.pop();
        // Cria um novo nó pai com frequência igual à soma das frequências dos filhos
        HuffmanNode *node = new HuffmanNode(L'\0', left->frequency + right->frequency);
        node->left = left;
        node->right = right;
        pq.push(node);  // Insere o novo nó na fila de prioridade
    }
    return pq.top();  // Retorna o nó raiz da árvore de Huffman
}

// Função para gerar os códigos de Huffman percorrendo a árvore
void generateCodes(HuffmanNode *root, const wstring &str, map<wchar_t, wstring> &codes) {
    if (!root) return;  // Se o nó for nulo, retorna
    if (!root->left && !root->right) {
        codes[root->symbol] = str;  // Armazena o código Huffman do símbolo quando chega a uma folha
    }
    generateCodes(root->left, str + L"0", codes);   // Gera o código para o filho esquerdo adicionando '0'
    generateCodes(root->right, str + L"1", codes);  // Gera o código para o filho direito adicionando '1'
}

// Função para exibir os códigos de Huffman
void displayCodes(const map<wchar_t, wstring> &codes, const map<wchar_t, int> &frequencies) {
    for (const auto& pair : codes) {
        wcout << pair.first << L": Codigo: " << pair.second << L", Frequencia: " << frequencies.at(pair.first) << endl;
    }
}

// Função para comprimir o arquivo usando os códigos de Huffman
wstring compressFile(const wstring &filename, const map<wchar_t, wstring> &codes) {
    // Converter wstring para string UTF-8
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
    string utf8_filename = converter.to_bytes(filename);

    // Abrir o arquivo usando wifstream para suportar caracteres wide
    wifstream wfile(utf8_filename);
    wfile.imbue(locale(wfile.getloc(), new codecvt_utf8<wchar_t>));

    // Verifica se o arquivo foi aberto corretamente
    if (!wfile.is_open()) {
        wcerr << L"Erro ao abrir o arquivo " << filename << endl;
        return L"";  // Retorna uma string vazia se não conseguir abrir o arquivo
    }

    wstring compressed;  // String para armazenar o arquivo comprimido
    wchar_t ch;
    // Lê cada caractere do arquivo e concatena seu código Huffman na string comprimida
    while (wfile.get(ch)) {
        compressed += codes.at(ch);
    }
    wfile.close();  // Fecha o arquivo após a leitura
    return compressed;  // Retorna a string comprimida
}

// Função para descomprimir o arquivo usando a árvore de Huffman
wstring decompressFile(const wstring &compressed, HuffmanNode* root) {
    wstring decompressed;  // String para armazenar o arquivo descomprimido
    HuffmanNode* current = root;  // Ponteiro para o nó atual na árvore de Huffman
    // Percorre cada bit na string comprimida para reconstruir o arquivo original
    for (wchar_t bit : compressed) {
        if (bit == L'0') {
            current = current->left;   // Se o bit for '0', move para o filho esquerdo
        } else {
            current = current->right;  // Se o bit for '1', move para o filho direito
        }
        // Quando alcança uma folha, adiciona o símbolo correspondente na string descomprimida
        if (!current->left && !current->right) {
            decompressed += current->symbol;
            current = root;  // Reinicia a busca a partir da raiz da árvore
        }
    }
    return decompressed;  // Retorna o arquivo descomprimido
}

// Função para comparar o tamanho do arquivo original com o tamanho comprimido
void displayFileSizeComparison(const wstring &filename, const map<wchar_t, int> &frequencies, const map<wchar_t, wstring> &codes) {
    // Converter wstring para string UTF-8
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
    string utf8_filename = converter.to_bytes(filename);

    // Tamanho original em bits
    ifstream file(utf8_filename, ios::binary | ios::ate);
    if (!file.is_open()) {
        wcerr << L"Erro ao abrir o arquivo " << filename << endl;
        return;  // Retorna se não conseguir abrir o arquivo
    }
    streamsize originalSize = file.tellg();  // Obtém o tamanho do arquivo em bytes
    file.close();
    int originalSizeBits = originalSize * 8;  // Converte para bits multiplicando por 8

    // Tamanho comprimido em bits
    int compressedSizeBits = 0;
    for (const auto& pair : frequencies) {
        compressedSizeBits += pair.second * codes.at(pair.first).length();  // Calcula o tamanho comprimido somando os tamanhos dos códigos Huffman
    }

    wcout << L"Tamanho original do arquivo: " << originalSizeBits << L" bits" << endl;
    wcout << L"Tamanho comprimido do arquivo: " << compressedSizeBits << L" bits" << endl;
    wcout << L"Diferenca: " << 100.0 * (originalSizeBits - compressedSizeBits) / originalSizeBits << L"%" << endl;
}

// Função para exportar a árvore de Huffman em formato DOT para o Graphviz
void export2dot(const HuffmanNode* root, const string& filename) {
    ofstream dot(filename);  // Abre o arquivo DOT para escrita
    dot << "digraph G {\n";  // Inicia o arquivo DOT para representar a árvore
    queue<const HuffmanNode*> q;
    q.push(root);

    // Percorre a árvore em largura para gerar o arquivo DOT
    while (!q.empty()) {
        const HuffmanNode* current = q.front();
        q.pop();

        current->printNode(dot);  // Imprime o nó atual no arquivo DOT

        // Adiciona os nós filhos à fila para processamento
        if (current->left) {
            dot << "\t" << reinterpret_cast<uintptr_t>(current) << " -> " << reinterpret_cast<uintptr_t>(current->left) << " [label=\"0\"];\n";
            q.push(current->left);
        }
        if (current->right) {
            dot << "\t" << reinterpret_cast<uintptr_t>(current) << " -> " << reinterpret_cast<uintptr_t>(current->right) << " [label=\"1\"];\n";
            q.push(current->right);
        }
    }
    dot << "}\n";  // Finaliza o arquivo DOT
}

// Função para desenhar a árvore de Huffman usando Graphviz
void draw(const HuffmanNode* root) {
    string dot_filename = "../saida/huffman_tree.dot";  // Nome do arquivo DOT para salvar a árvore
    string dot_executable = "\"C:\\Program Files\\Graphviz\\bin\\dot\"";  // Caminho para o executável dot do Graphviz

    string command = dot_executable + " -Tpng " + dot_filename + " -o ../saida/graph.png";  // Comando para gerar a imagem PNG da árvore

#ifdef _WIN32
    int ret = _mkdir("../saida");  // Cria o diretório de saída no Windows se não existir
#else
    mode_t mode = 0755;
    int ret = mkdir("../saida", mode);  // Cria o diretório de saída em sistemas Unix-like se não existir
#endif
    if (ret != 0 && errno != EEXIST) {
        cerr << "Erro ao criar o diretorio ../saida/" << endl;
        return;  // Retorna se houver erro ao criar o diretório de saída
    }

    export2dot(root, dot_filename);  // Exporta a árvore de Huffman para o arquivo DOT
    system(command.c_str());  // Executa o comando para gerar a imagem PNG da árvore usando o Graphviz
}

// Função principal
int main() {
    wcout << L"Digite o nome do arquivo de texto (exemplo: arquivo.txt): ";
    wstring filename;
    wcin >> filename;  // Lê o nome do arquivo fornecido pelo usuário

    // Construir o caminho completo para o arquivo na pasta public
    wstring fullPath = L"../../public/" + filename;

    // Abrir arquivo de saída para redirecionar wcout
    wofstream outputFile("../saida/texto.txt");
    wstreambuf *coutbuf = wcout.rdbuf();  // Salvar buffer original de wcout
    wcout.rdbuf(outputFile.rdbuf());  // Redirecionar wcout para outputFile

    map<wchar_t, int> frequencies = analyzeFile(fullPath);  // Analisa o arquivo para calcular as frequências dos caracteres

    if (frequencies.empty()) {
        wcerr << L"Erro: Nao foi possivel ler o arquivo ou o arquivo esta vazio." << endl;
        return 1;  // Retorna se houver erro ao ler o arquivo ou se o arquivo estiver vazio
    }

    HuffmanNode* root = buildHuffmanTree(frequencies);  // Constrói a árvore de Huffman a partir das frequências

    map<wchar_t, wstring> codes;  // Mapa para armazenar os códigos de Huffman
    generateCodes(root, L"", codes);  // Gera os códigos de Huffman para cada símbolo na árvore

    draw(root);  // Desenha a árvore de Huffman e gera a imagem PNG correspondente

    wcout << L"\nComparacao dos tamanhos dos arquivos:\n";
    displayFileSizeComparison(fullPath, frequencies, codes);  // Exibe a comparação entre os tamanhos do arquivo original e comprimido

    wcout << L"\nCodigos de Huffman e Frequencias dos Caracteres:\n";
    displayCodes(codes, frequencies);  // Exibe os códigos de Huffman gerados e suas frequências correspondentes

    // Restaurar buffer original de wcout e fechar arquivo de saída
    wcout.rdbuf(coutbuf);
    outputFile.close();

    return 0;  // Retorna 0 indicando que o programa foi executado com sucesso
}
