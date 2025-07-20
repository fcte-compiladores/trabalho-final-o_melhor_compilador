// test.c
// Testes para as Atividades 1, 2 e 3

// Atividade 1: Nova forma de declarar variáveis
$minhaVariavel=int;
$outraVariavel=float;
$terceira=char;

// Atividade 2: Nova forma de declarar structs
// Sintaxe tradicional (para garantir que ainda funciona)
struct Ponto {
    int x;
    int y;
};

// NOVA SINTAXE DE STRUCT: struct NomeStruct = Tipo Membro1, Tipo Membro2;
struct Retangulo = int largura, int altura, float area;
struct Coordenada3D=double x, double y, double z;

// Atividade 3: Nova forma de declarar if/else

int main() {
    int a = 10;
    int b = 20;
    int c = 30;

    // NOVO: if Expression: Statement;
    // Exemplo: if 1 > 0: return 1;
    if (a > b) { 
        return 1;
    };

    // NOVO: if Expression: Statement: Statement;
    // Exemplo: if 1 > 2: return 2: return 3;
    if (b > c) { 
        return 2;
    } else {
        return 3;
    };

    // NOVO: if Expression} return 2: return 3;

    // Exemplo para garantir que o if tradicional ainda funciona (se for implementado mais tarde)
    // if (a == 10) {
    //    return 4;
    // } else {
    //    return 5;
    // }

    return 0;
}