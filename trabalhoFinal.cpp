#include <iostream>
#include <fstream>
#include <string>
#include <map>

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

int main() {
    wstring filename = "input.txt";
    map<wchar_t, int> frequencies = analyzeFile(filename);
    for (const auto& pair : frequencies) {
        wcout << pair.first << L": " << pair.second << endl;
    }
    return 0;
}
