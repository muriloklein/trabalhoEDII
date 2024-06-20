#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <queue>
#include <locale>
#include <codecvt>
#include <cstdlib>
#include <sys/stat.h>
#include <cerrno>

#ifdef _WIN32
#include <direct.h>
#endif

using namespace std;

// Função para analisar o arquivo e calcular as frequências de cada caractere
map<wchar_t, int> analyzeFile(const wstring &filename) {
    // Converter wstring para string UTF-8
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
    string utf8_filename = converter.to_bytes(filename);

    // Abrir o arquivo usando ifstream com string UTF-8
    ifstream file(utf8_filename, ios::binary);
    if (!file.is_open()) {
        wcerr << L"Erro ao abrir o arquivo " << filename << endl;
        return {};
    }

    // Ler o arquivo usando wifstream para suportar caracteres wide
    wifstream wfile(utf8_filename);
    wfile.imbue(locale(wfile.getloc(), new codecvt_utf8_utf16<wchar_t>));

    map<wchar_t, int> frequencies;
    wchar_t ch;
    while (wfile.get(ch)) {
        frequencies[ch]++;
    }
    wfile.close();
    return frequencies;
}

// Estrutura para representar um nó na árvore de Huffman
struct HuffmanNode {
    wchar_t symbol;
    int frequency;
    HuffmanNode *left;
    HuffmanNode *right;
    HuffmanNode(wchar_t s, int f) : symbol(s), frequency(f), left(nullptr), right(nullptr) {}

    void printNode(ofstream& dot) const {
        dot << "\t" << reinterpret_cast<uintptr_t>(this) << " [label=\"" << frequency << "\"];\n";
    }
};

// Functor para comparar dois nós na fila de prioridade
struct compare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return (l->frequency > r->frequency);
    }
};

// Função para construir a árvore de Huffman a partir das frequências dos caracteres
HuffmanNode* buildHuffmanTree(const map<wchar_t, int> &frequencies) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, compare> pq;
    for (auto pair : frequencies) {
        pq.push(new HuffmanNode(pair.first, pair.second));
    }
    while (pq.size() > 1) {
        HuffmanNode *left = pq.top(); pq.pop();
        HuffmanNode *right = pq.top(); pq.pop();
        HuffmanNode *node = new HuffmanNode(L'\0', left->frequency + right->frequency);
        node->left = left;
        node->right = right;
        pq.push(node);
    }
    return pq.top();
}

// Função para gerar os códigos de Huffman percorrendo a árvore
void generateCodes(HuffmanNode *root, const wstring &str, map<wchar_t, wstring> &codes) {
    if (!root) return;
    if (!root->left && !root->right) {
        codes[root->symbol] = str;
    }
    generateCodes(root->left, str + L"0", codes);
    generateCodes(root->right, str + L"1", codes);
}

// Função para exibir os códigos de Huffman
void displayCodes(const map<wchar_t, wstring> &codes) {
    for (auto pair : codes) {
        wcout << pair.first << L": " << pair.second << endl;
    }
}

// Função para comprimir o arquivo usando os códigos de Huffman
wstring compressFile(const wstring &filename, const map<wchar_t, wstring> &codes) {
    // Converter wstring para string UTF-8
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
    string utf8_filename = converter.to_bytes(filename);

    // Abrir o arquivo usando ifstream com string UTF-8
    ifstream file(utf8_filename, ios::binary);
    if (!file.is_open()) {
        wcerr << L"Erro ao abrir o arquivo " << filename << endl;
        return L"";
    }

    // Ler o arquivo usando wifstream para suportar caracteres wide
    wifstream wfile(utf8_filename);
    wfile.imbue(locale(wfile.getloc(), new codecvt_utf8_utf16<wchar_t>));

    wstring compressed;
    wchar_t ch;
    while (wfile.get(ch)) {
        compressed += codes.at(ch);
    }
    wfile.close();
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
        originalSize += pair.second * sizeof(wchar_t) * 8;
        compressedSize += pair.second * codes.at(pair.first).length();
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
    string dot_executable = "\"C:\\Program Files\\Graphviz\\bin\\dot\""; // Coloque entre aspas

    string command = dot_executable + " -Tpng " + dot_filename + " -o ../saida/graph.png";

#ifdef _WIN32
    int ret = _mkdir("../saida");
#else
    mode_t mode = 0755;
    int ret = mkdir("../saida", mode);
#endif
    if (ret != 0 && errno != EEXIST) {
        cerr << "Erro ao criar o diretório ../saida/" << endl;
        return;
    }

    export2dot(root, dot_filename);
    system(command.c_str());
}

int main() {
    wcout << L"Digite o nome do arquivo de texto (exemplo: arquivo.txt): ";
    wstring filename;
    wcin >> filename;

    // Construir o caminho completo para o arquivo na pasta public
    wstring fullPath = L"../../public/" + filename;

    map<wchar_t, int> frequencies = analyzeFile(fullPath);

    if (frequencies.empty()) {
        wcerr << L"Erro: Não foi possível ler o arquivo ou o arquivo está vazio." << endl;
        return 1;
    }

    wcout << L"\nFrequências dos caracteres:\n";
    for (const auto& pair : frequencies) {
        wcout << pair.first << L": " << pair.second << endl;
    }

    HuffmanNode* root = buildHuffmanTree(frequencies);

    map<wchar_t, wstring> codes;
    generateCodes(root, L"", codes);

    wcout << L"\nCódigos de Huffman:\n";
    displayCodes(codes);

    wcout << L"\nComparação de tamanhos:\n";
    displayFileSizeComparison(frequencies, codes);

    draw(root);

    return 0;
}