#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"
#include "helpers/vector.h"

void print_tokens(struct vector* token_vec) {
    if (!token_vec || !token_vec->data) {
        printf("Erro: Vetor de tokens inválido\n");
        return;
    }

    printf("\n=== Tokens Gerados ===\n");
    vector_set_peek_pointer(token_vec, 0);
    struct token* token = vector_peek(token_vec);
    
    while(token) {
        if (!token) {
            printf("TOKEN_NULO ");
            token = vector_peek(token_vec);
            continue;
        }

        switch(token->type) {
            case TOKEN_TYPE_KEYWORD: 
                printf("KEYWORD(%s) ", token->sval ? token->sval : "NULL");
                break;
            case TOKEN_TYPE_IDENTIFIER: 
                printf("ID(%s) ", token->sval ? token->sval : "NULL");
                break;
            case TOKEN_TYPE_OPERATOR:
                printf("OP(%s) ", token->sval ? token->sval : "NULL_OP");
                break;
            case TOKEN_TYPE_SYMBOL:
                printf("SYM(%c) ", token->cval);
                break;
            case TOKEN_TYPE_NUMBER:
                printf("NUM(%llu) ", token->llnum); // Alterado para %llu para unsigned long long
                break;
            case TOKEN_TYPE_STRING:
                printf("STR(%s) ", token->sval ? token->sval : "NULL");
                break;
            case TOKEN_TYPE_COMMENT: 
                printf("COMMENT ");
                break;
            case TOKEN_TYPE_NEWLINE:
                printf("NL ");
                break;
            case TOKEN_TYPE_DOLLAR_SIGN: 
                printf("DOLLAR_SIGN(%c) ", token->cval);
                break;
            default:
                printf("UNKNOWN_TYPE_%d ", token->type);
        }
        token = vector_peek(token_vec);
    }
    printf("\n");
}

int main(int argc, char const *argv[]) {
    const char* input_file = "test.c"; 
    printf("# Iniciando compilação de %s\n", input_file); 

    // Verificação do arquivo
    FILE* test_file = fopen(input_file, "r"); 
    if (!test_file) { 
        printf("Erro: Não foi possível abrir %s\n", input_file); 
        return 1; 
    }
    fclose(test_file); 

    // Criação do processo
    struct compile_process* process = compile_process_create(input_file, NULL, 0); 
    if (!process) { 
        printf("Falha ao criar processo de compilação\n"); 
        return 1; 
    }

    // Configura as funções do lex_process
    struct lex_process_functions functions = { 
        .next_char = compile_process_next_char, 
        .peek_char = compile_process_peek_char, 
        .push_char = compile_process_push_char 
    };

    // Criação do processo léxico
    struct lex_process* lex_process = lex_process_create(process, &functions, NULL); 
    if (!lex_process) { 
        printf("Falha ao criar processo léxico\n"); 
        compile_process_free(process); 
        return 1; 
    }

    // Análise léxica
    if (lex(lex_process) != LEXICAL_ANALYSIS_ALL_OK) { 
        printf("Falha na análise léxica\n"); 
        lex_process_free(lex_process); 
        compile_process_free(process); 
        return 1; 
    }

    // Imprime tokens para debug
    printf("Total de tokens: %d\n", vector_count(lex_process->token_vec)); 
    print_tokens(lex_process->token_vec); 

    // Análise sintática
    if (parse(process) != PARSE_ALL_OK) { 
         printf("Falha na análise sintática\n"); 
         lex_process_free(lex_process); 
         compile_process_free(process); 
         return 1; 
    }

    printf("# Compilação concluída com sucesso\n"); 
    
    // Liberação de recursos
    lex_process_free(lex_process); 
    compile_process_free(process); 
    return 0; 
}