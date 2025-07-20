# Compilador LOGO para SVG

## Integrantes

- Vitor Valerio Hoffmann - 222006211 - turma 16h
- Ana Luiza Pfeilsticker de Oliveira Araujo Tristão - 222015060 - turma 16h
- Atyrson Souto da Silva - 251005945 - turma 16h

## Introdução

Este projeto apresenta um compilador para uma linguagem inspirada no LOGO, projetada para gerar imagens vetoriais no formato SVG. LOGO, uma linguagem de programação educacional, é famosa por sua abordagem de "turtle graphics", que permite aos usuários criar imagens complexas através de comandos simples que controlam um cursor gráfico, a "tartaruga".

Nosso compilador reinterpreta essa ideia clássica, permitindo que desenvolvedores e entusiastas criem gráficos vetoriais de alta qualidade de forma programática. A implementação, desenvolvida em JavaScript, segue um pipeline de compilação clássico:

1.  **Análise Léxica (Tokenizer):** O processo começa com o `tokenizer`, que converte o código-fonte LOGO em uma série de tokens atômicos, como comandos (`FORWARD`), números (`100`) e símbolos (`[`).
2.  **Análise Sintática (Parser):** Em seguida, o `parser` consome esses tokens para construir uma Árvore de Sintaxe Abstrata (AST). A AST é uma representação hierárquica da estrutura do código, essencial para a próxima fase.
3.  **Interpretação (Interpreter):** O `interpreter` atravessa a AST, executando as instruções de forma semântica. Ele simula o comportamento da tartaruga, calculando as coordenadas e os caminhos que formarão a imagem final, mas sem renderizar diretamente. Em vez disso, ele cria uma representação de dados intermediária.
4.  **Geração de Código (Generator):** Finalmente, o `generator` utiliza essa representação intermediária para produzir o código SVG final. O resultado é um arquivo `.svg` limpo e otimizado, que pode ser visualizado em qualquer navegador moderno ou editor de imagens vetoriais.

Este projeto não apenas serve como um exercício prático nos princípios da construção de compiladores, mas também oferece uma ferramenta divertida e poderosa para a criação de arte generativa e visualizações de dados.

### Comandos Suportados

A linguagem suporta os seguintes comandos:

- `FORWARD <dist>` ou `FD <dist>`: Move a tartaruga para a frente pela distância especificada.
- `BACK <dist>` ou `BK <dist>`: Move a tartaruga para trás pela distância especificada.
- `RIGHT <angle>` ou `RT <angle>`: Vira a tartaruga para a direita pelo ângulo especificado.
- `LEFT <angle>` ou `LT <angle>`: Vira a tartaruga para a esquerda pelo ângulo especificado.
- `PENUP` ou `PU`: Levanta a caneta, de modo que a tartaruga não desenhe ao se mover.
- `PENDOWN` ou `PD`: Abaixa a caneta, de modo que a tartaruga desenhe ao se mover.
- `BACKGROUND <color>` ou `BG <color>`: Define a cor de fundo da imagem. A cor pode ser um nome de cor em inglês (ex: `red`, `blue`) ou um código hexadecimal (ex: `#FF0000`).
- `REPEAT <count> [ <commands> ]`: Repete uma lista de comandos um determinado número de vezes.
- `CIRCLE <radius>`: Desenha um círculo com o raio especificado.
- `RECTANGLE <width> <height>`: Desenha um retângulo com a largura e altura especificadas.
- `POLYGON <sides> <size>`: Desenha um polígono regular com o número de lados e tamanho especificados.
- `CURVE <x1> <y1> <x2> <y2> <x3> <y3>`: Desenha uma curva de Bézier.

## Instalação

Para usar o compilador, você precisa ter o Node.js instalado. Em seguida, clone o repositório:

```bash
git clone https://github.com/fcte-compiladores/trabalho-final-o_melhor_compilador.git
cd trabalho-final-o_melhor_compilador
```

Para compilar um arquivo LOGO para SVG, use o seguinte comando:

```bash
npm start exemplos/meu-arquivo.logo
```

Isso irá gerar um arquivo `meu-arquivo.svg` no diretório raiz.

## Exemplos

A pasta `exemplos` contém vários exemplos de código LOGO, desde formas simples até desenhos mais complexos.

- `quadrado.logo`: Desenha um quadrado.
- `hexagono.logo`: Desenha um hexágono.
- `estrela.logo`: Desenha uma estrela.
- `flor.logo`: Desenha uma flor usando o comando `REPEAT`.
- `galaxia.logo`: Desenha uma galáxia usando o comando `REPEAT` e `CURVE`.

etc...

## Estrutura do Código

O código-fonte está localizado na pasta `src` e está dividido nos seguintes módulos:

- `tokenizer.js`: Contém a função `tokenize`, que realiza a análise léxica.
- `parser.js`: Contém a função `parse`, que realiza a análise sintática e constrói a AST.
- `interpreter.js`: Contém a função `interpret`, que executa as instruções da AST.
- `generator.js`: Contém a função `generateSVG`, que gera o código SVG final.
- `logo2svg.js`: O script principal que lê um arquivo LOGO, o compila e salva o resultado em um arquivo SVG.

## Bugs/Limitações/Problemas Conhecidos

- O tratamento de erros é básico. Erros de sintaxe ou comandos desconhecidos podem não ser reportados de forma clara.
- A linguagem não suporta variáveis, procedimentos ou outras estruturas de controle mais avançadas.
- A linguagem não aceita comentários.
- A linguagem atual não possui funcionalidade de animação (gerar gif's etc...).

## Referências

### Linguagem Logo & Turtle Graphics

- Papert, Seymour. _Mindstorms: Children, Computers, and Powerful Ideas_. Basic Books, 1980.
- Wikipedia: “Logo (programming language)”. Disponível em: https://en.wikipedia.org/wiki/Logo_(programming_language)
- Wikipedia: “Turtle graphics”. Disponível em: https://en.wikipedia.org/wiki/Turtle_graphics

### Compiladores

- Aho, Alfred V.; Sethi, Ravi; Ullman, Jeffrey D. _Compilers: Principles, Techniques, and Tools_ (2ª ed.). Addison-Wesley, 2006.

### SVG

- W3C. _Scalable Vector Graphics (SVG) 1.1 Specification_. Disponível em: https://www.w3.org/TR/SVG11/
- MDN Web Docs: “SVG `<circle>` element”. Disponível em: https://developer.mozilla.org/docs/Web/SVG/Element/circle
- MDN Web Docs: “SVG `<rect>` element”. Disponível em: https://developer.mozilla.org/docs/Web/SVG/Element/rect
- MDN Web Docs: “SVG `<path>` element”. Disponível em: https://developer.mozilla.org/docs/Web/SVG/Element/path

### Gramáticas

- ISO/IEC 14977. _Extended BNF (EBNF) — The ISO Standard for Syntax Notation_.
- Wikipedia: “Extended Backus–Naur form”. Disponível em: https://en.wikipedia.org/wiki/Extended_Backus–Naur_form

### Curvas Bézier

- Wikipedia: “Bézier curve”. Disponível em: https://en.wikipedia.org/wiki/B%C3%A9zier_curve
