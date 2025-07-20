/* BEGIN - LAB 2 ---------------------------------*/
#include "compiler.h"
#include "helpers/vector.h"
#include <stdlib.h>

struct lex_process* lex_process_create(struct compile_process* compiler, struct lex_process_functions* functions, void *private) {
    struct lex_process* process = calloc(1, sizeof(struct lex_process));
    if (!process) return NULL;

    process->compiler = compiler;
    process->function = functions;
    process->private = private;
    process->pos.filename = compiler->cfile.abs_path;
    
    // Use o vetor de tokens do compile_process
    process->token_vec = compiler->token_vec;
    if (!process->token_vec) {
        free(process);
        return NULL;
    }

    return process;
}

void lex_process_free(struct lex_process* process) {
    if (!process) return;
    
    if (process->token_vec) {
        // Libera cada token individualmente se necessário
        vector_free(process->token_vec);
    }
    
    if (process->parentheses_buffer) {
        buffer_free(process->parentheses_buffer);
    }
    
    free(process);
}
void* lex_process_private(struct lex_process* process) {
    return process->private;
}

struct vector* lex_process_tokens(struct lex_process* process){
    return process->token_vec;
}
/* END - LAB 2 ---------------------------------*/


