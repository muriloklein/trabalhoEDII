# Codificação de Huffman - Trabalho Final de Estruturas de Dados 2

### Componentes:

- Murilo Kaemmerer Klein
- Matheus Costa Curta

## Introdução

No mundo da compressão de dados, a codificação de Huffman se destaca como um algoritmo eficiente criado por David A. Huffman em 1952. Este método atribui códigos binários mais curtos aos símbolos que aparecem com mais frequência e códigos binários mais longos aos símbolos menos frequentes, reduzindo o tamanho geral do arquivo e otimizando o espaço de armazenamento e a transmissão de dados. A codificação de Huffman é utilizada em diversas aplicações, como zip, jpeg, png, mp3, entre outros.

## Objetivo

Este projeto tem como objetivo implementar um software em C++ que utilize árvores de Huffman para comprimir e descompactar arquivos de texto. O software deve ser capaz de:

1. Ler e analisar arquivos de texto para extrair as frequências de cada símbolo.
2. Construir uma árvore de Huffman a partir das frequências dos símbolos.
3. Gerar uma tabela de codificação atribuindo códigos binários únicos para cada símbolo.
4. Exibir a árvore de Huffman e a tabela de codificação de forma amigável.
5. Comparar o tamanho do arquivo original com o tamanho comprimido e calcular a taxa de compressão.

## Funcionalidades

### Análise de Frequências

A função `analyzeFile` lê um arquivo de texto e calcula as frequências de cada caractere Unicode, retornando um mapa de frequências.

### Construção da Árvore de Huffman

A função `buildHuffmanTree` utiliza uma fila de prioridade para construir a árvore de Huffman a partir das frequências dos caracteres.

### Geração de Códigos

A função `generateCodes` percorre a árvore de Huffman para gerar os códigos binários correspondentes a cada símbolo.

### Exibição dos Resultados

As funções `displayCodes` e `displayFileSizeComparison` exibem os códigos de Huffman e comparam o tamanho do arquivo original com o tamanho comprimido, respectivamente.

## Como Executar

1. Compile o código utilizando um compilador C++ compatível com o padrão C++11 ou superior.
2. Execute o programa fornecendo o nome do arquivo de texto a ser analisado.
3. O programa exibirá as frequências dos caracteres, os códigos de Huffman gerados e a comparação do tamanho dos arquivos.