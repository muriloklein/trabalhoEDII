#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <queue>

using namespace std;

map<wchar_t, int> analyzeFile(const wstring &filename) {
    wifstream file(filename);
    map<wchar_t, int> frequencies;
    wchar_t ch;
    while (file.get(ch)) {
        frequencies[ch]++;
    }
    return frequencies;
}

struct HuffmanNode {
    wchar_t symbol;
    int frequency;
    HuffmanNode *left;
    HuffmanNode *right;
    HuffmanNode(wchar_t s, int f) : symbol(s), frequency(f), left(nullptr), right(nullptr) {}
};

struct compare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return (l->frequency > r->frequency);
    }
};

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

int main() {
    wstring filename = "input.txt";
    map<wchar_t, int> frequencies = analyzeFile(filename);
    for (const auto& pair : frequencies) {
        wcout << pair.first << L": " << pair.second << endl;
    }
    return 0;
}
