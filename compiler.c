#include <stdio.h>
#include <stdlib.h>
#include "helpers/vector.h"
#include "compiler.h"
#include <stdarg.h> 

struct compile_process* current_process = NULL;
struct token* parser_last_token = NULL;

void compiler_error(struct compile_process* compiler, const char* msg, ...) {
    va_list args;
    va_start(args, msg);
    
    fprintf(stderr, "Erro: ");
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    
    va_end(args);
}

struct compile_process* compile_process_create(const char* filename, const char* filename_out, int flags) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    struct compile_process* process = calloc(1, sizeof(struct compile_process));
    if (!process) {
        fclose(file);
        return NULL;
    }

    process->flags = flags;
    process->cfile.fp = file;
    process->cfile.abs_path = filename;

    process->token_vec = vector_create(sizeof(struct token));
    process->node_vec = vector_create(sizeof(struct node*));
    process->node_tree_vec = vector_create(sizeof(struct node*));

    if (!process->token_vec || !process->node_vec || !process->node_tree_vec) {
        if (process->token_vec) vector_free(process->token_vec);
        if (process->node_vec) vector_free(process->node_vec);
        if (process->node_tree_vec) vector_free(process->node_tree_vec);
        fclose(file);
        free(process);
        return NULL;
    }

    return process;
}

void compile_process_free(struct compile_process* process) {
    if (!process) return;

    if (process->token_vec) vector_free(process->token_vec);
    if (process->node_vec) vector_free(process->node_vec);
    if (process->node_tree_vec) vector_free(process->node_tree_vec);
    if (process->cfile.fp) fclose(process->cfile.fp);
    
    free(process);
}