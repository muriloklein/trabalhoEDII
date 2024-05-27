#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <queue>

using namespace std;

// Função para analisar o arquivo e calcular as frequências de cada caractere
map<wchar_t, int> analyzeFile(const wstring &filename) {
    wifstream file(filename);  // Abre o arquivo para leitura
    map<wchar_t, int> frequencies;  // Mapa para armazenar as frequências dos caracteres
    wchar_t ch;
    while (file.get(ch)) {  // Lê cada caractere do arquivo
        frequencies[ch]++;  // Incrementa a frequência do caractere lido
    }
    return frequencies;  // Retorna o mapa de frequências
}

// Estrutura para representar um nó na árvore de Huffman
struct HuffmanNode {
    wchar_t symbol;  // Símbolo armazenado no nó
    int frequency;  // Frequência do símbolo
    HuffmanNode *left;  // Ponteiro para o filho esquerdo
    HuffmanNode *right;  // Ponteiro para o filho direito
    HuffmanNode(wchar_t s, int f) : symbol(s), frequency(f), left(nullptr), right(nullptr) {}  // Construtor do nó
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
void generateCodes(HuffmanNode *root, const std::wstring &str, std::map<wchar_t, std::wstring> &codes) {
    if (!root) return;
    if (!root->left && !root->right) {  // Verifica se é uma folha
        codes[root->symbol] = str;  // Armazena o código gerado para o símbolo
    }
    generateCodes(root->left, str + L"0", codes);  // Percorre a subárvore esquerda
    generateCodes(root->right, str + L"1", codes);  // Percorre a subárvore direita
}

// Função para exibir os códigos de Huffman
void displayCodes(const std::map<wchar_t, std::wstring> &codes) {
    for (auto pair : codes) {
        std::wcout << pair.first << L": " << pair.second << std::endl;  // Exibe cada símbolo e seu código
    }
}

// Função para comparar o tamanho do arquivo original com o tamanho comprimido
void displayFileSizeComparison(const std::map<wchar_t, int> &frequencies, const std::map<wchar_t, std::wstring> &codes) {
    int originalSize = 0;
    int compressedSize = 0;
    for (auto pair : frequencies) {
        originalSize += pair.second * sizeof(wchar_t) * 8;  // Calcula o tamanho original em bits
        compressedSize += pair.second * codes.at(pair.first).length();  // Calcula o tamanho comprimido em bits
    }
    std::wcout << L"Original Size: " << originalSize << L" bits" << std::endl;
    std::wcout << L"Compressed Size: " << compressedSize << L" bits" << std::endl;
    std::wcout << L"Compression Ratio: " << 100.0 * (originalSize - compressedSize) / originalSize << L"%" << std::endl;  // Exibe a taxa de compressão
}

int main() {
    wstring filename = L"input.txt";  // Nome do arquivo a ser analisado
    map<wchar_t, int> frequencies = analyzeFile(filename);  // Análise das frequências dos caracteres no arquivo
    for (const auto& pair : frequencies) {
        wcout << pair.first << L": " << pair.second << endl;  // Exibe as frequências dos caracteres
    }
    return 0;
}
