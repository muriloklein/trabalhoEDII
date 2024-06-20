#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <queue>
#include <locale>
#include <codecvt>
#include <cstdlib>
#include <sys/stat.h>   // Para mkdir
#include <errno.h>      // Para tratar erros de criação de diretório

using namespace std;

// Função para analisar o arquivo e calcular as frequências de cada caractere
map<wchar_t, int> analyzeFile(const wstring &filename) {
    wifstream file(filename.c_str());  // Abre o arquivo para leitura
    if (!file.is_open()) {
        wcerr << L"Erro ao abrir o arquivo " << filename << endl;
        return {};
    }
    file.imbue(locale(file.getloc(), new codecvt_utf8<wchar_t>));  // Define a locação para suportar UTF-8
    map<wchar_t, int> frequencies;  // Mapa para armazenar as frequências dos caracteres
    wchar_t ch;
    while (file.get(ch)) {  // Lê cada caractere do arquivo
        frequencies[ch]++;  // Incrementa a frequência do caractere lido
    }
    file.close();
    return frequencies;  // Retorna o mapa de frequências
}

// Estrutura para representar um nó na árvore de Huffman
struct HuffmanNode {
    wchar_t symbol;  // Símbolo armazenado no nó
    int frequency;  // Frequência do símbolo
    HuffmanNode *left;  // Ponteiro para o filho esquerdo
    HuffmanNode *right;  // Ponteiro para o filho direito
    HuffmanNode(wchar_t s, int f) : symbol(s), frequency(f), left(nullptr), right(nullptr) {}  // Construtor do nó

    // Método para imprimir o nó no formato do Graphviz DOT
    void printNode(ofstream& dot) const {
        dot << "\t" << reinterpret_cast<uintptr_t>(this) << " [label=\"" << frequency << "\"];\n";
    }
};

// Functor para comparar dois nós na fila de prioridade
struct compare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return (l->frequency > r->frequency);  // Compara os nós pela frequência (menor frequência tem maior prioridade)
    }
};

// Função para construir a árvore de Huffman a partir das frequências dos caracteres
HuffmanNode* buildHuffmanTree(const map<wchar_t, int> &frequencies) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, compare> pq;  // Fila de prioridade para construir a árvore
    for (auto pair : frequencies) {
        pq.push(new HuffmanNode(pair.first, pair.second));  // Insere todos os nós na fila de prioridade
    }
    while (pq.size() > 1) {
        HuffmanNode *left = pq.top(); pq.pop();  // Remove o nó com menor frequência
        HuffmanNode *right = pq.top(); pq.pop();  // Remove o próximo nó com menor frequência
        HuffmanNode *node = new HuffmanNode(L'\0', left->frequency + right->frequency);  // Cria um novo nó combinando as frequências
        node->left = left;  // Define o filho esquerdo
        node->right = right;  // Define o filho direito
        pq.push(node);  // Insere o novo nó na fila de prioridade
    }
    return pq.top();  // Retorna o nó raiz da árvore de Huffman
}

// Função para gerar os códigos de Huffman percorrendo a árvore
void generateCodes(HuffmanNode *root, const wstring &str, map<wchar_t, wstring> &codes) {
    if (!root) return;
    if (!root->left && !root->right) {  // Verifica se é uma folha
        codes[root->symbol] = str;  // Armazena o código gerado para o símbolo
    }
    generateCodes(root->left, str + L"0", codes);  // Percorre a subárvore esquerda
    generateCodes(root->right, str + L"1", codes);  // Percorre a subárvore direita
}

// Função para exibir os códigos de Huffman
void displayCodes(const map<wchar_t, wstring> &codes) {
    for (auto pair : codes) {
        wcout << pair.first << L": " << pair.second << endl;  // Exibe cada símbolo e seu código
    }
}

// Função para comprimir o arquivo usando os códigos de Huffman
wstring compressFile(const wstring &filename, const map<wchar_t, wstring> &codes) {
    wifstream file(filename.c_str());  // Abre o arquivo para leitura
    if (!file.is_open()) {
        wcerr << L"Erro ao abrir o arquivo " << filename << endl;
        return L"";
    }
    file.imbue(locale(file.getloc(), new codecvt_utf8<wchar_t>));  // Define a locação para suportar UTF-8
    wstring compressed;
    wchar_t ch;
    while (file.get(ch)) {
        compressed += codes.at(ch);  // Adiciona o código do caractere à string comprimida
    }
    file.close();
    return compressed;
}

// Função para descomprimir o arquivo usando a árvore de Huffman
wstring decompressFile(const wstring &compressed, HuffmanNode* root) {
    wstring decompressed;
    HuffmanNode* current = root;
    for (wchar_t bit : compressed) {
        if (bit == L'0') {
            current = current->left;
        } else {
            current = current->right;
        }
        if (!current->left && !current->right) {
            decompressed += current->symbol;
            current = root;
        }
    }
    return decompressed;
}

// Função para comparar o tamanho do arquivo original com o tamanho comprimido
void displayFileSizeComparison(const map<wchar_t, int> &frequencies, const map<wchar_t, wstring> &codes) {
    int originalSize = 0;
    int compressedSize = 0;
    for (auto pair : frequencies) {
        originalSize += pair.second * sizeof(wchar_t) * 8;  // Calcula o tamanho original em bits
        compressedSize += pair.second * codes.at(pair.first).length();  // Calcula o tamanho comprimido em bits
    }
    wcout << L"Tamanho original do arquivo: " << originalSize << L" bits" << endl;
    wcout << L"Tamanho comprimido do arquivo: " << compressedSize << L" bits" << endl;
    wcout << L"Diferença: " << 100.0 * (originalSize - compressedSize) / originalSize << L"%" << endl;
}

// Função para exportar a árvore de Huffman em formato DOT para o Graphviz
void export2dot(const HuffmanNode* root, const string& filename) {
    ofstream dot(filename);
    dot << "digraph G {\n";
    queue<const HuffmanNode*> q;
    q.push(root);

    while (!q.empty()) {
        const HuffmanNode* current = q.front();
        q.pop();

        current->printNode(dot);

        if (current->left) {
            dot << "\t" << reinterpret_cast<uintptr_t>(current) << " -> " << reinterpret_cast<uintptr_t>(current->left) << " [label=\"0\"];\n";
            q.push(current->left);
        }
        if (current->right) {
            dot << "\t" << reinterpret_cast<uintptr_t>(current) << " -> " << reinterpret_cast<uintptr_t>(current->right) << " [label=\"1\"];\n";
            q.push(current->right);
        }
    }
    dot << "}\n";
}

// Função para desenhar a árvore de Huffman usando Graphviz
void draw(const HuffmanNode* root) {
    string dot_filename = "../saida/huffman_tree.dot";
    string command = "dot -Tpng " + dot_filename + " -o ../saida/graph.png";

    // Criar o diretório de saída se não existir
    #ifdef _WIN32
    int ret = mkdir("../saida");
    #else
    mode_t mode = 0755;  // Permissões padrão para criar diretório
    int ret = mkdir("../saida", mode);
    #endif
    if (ret != 0 && errno != EEXIST) {
        cerr << "Erro ao criar o diretório ../saida/" << endl;
        return;
    }

    // Exportar árvore de Huffman para o arquivo DOT
    export2dot(root, dot_filename);

    // Executar o comando para gerar a imagem PNG da árvore
    system(command.c_str());
}

int main() {
    // Leitura do nome do arquivo a partir da entrada do usuário
    wcout << L"Digite o nome do arquivo de texto: ";
    wstring filename;
    wcin >> filename;

    // Análise das frequências dos caracteres no arquivo
    map<wchar_t, int> frequencies = analyzeFile(filename);

    // Verificação se o arquivo foi lido corretamente
    if (frequencies.empty()) {
        wcerr << L"Erro: Não foi possível ler o arquivo ou o arquivo está vazio." << endl;
        return 1;
    }

    // Exibição das frequências dos caracteres
    wcout << L"\nFrequências dos caracteres:\n";
    for (const auto& pair : frequencies) {
        wcout << pair.first << L": " << pair.second << endl;
    }

    // Construção da árvore de Huffman
    HuffmanNode* root = buildHuffmanTree(frequencies);

    // Geração dos códigos de Huffman
    map<wchar_t, wstring> codes;
    generateCodes(root, L"", codes);

    // Exibição dos códigos de Huffman
    wcout << L"\nCódigos de Huffman:\n";
    displayCodes(codes);

    // Exibição da comparação do tamanho dos arquivos
    wcout << L"\nComparação de tamanhos:\n";
    displayFileSizeComparison(frequencies, codes);

    // Desenho da árvore de Huffman usando Graphviz
    draw(root);

    return 0;
}
