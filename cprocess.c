#include "compiler.h"
#include "helpers/vector.h"  
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h> 

extern struct compile_process* current_process;
extern struct token* parser_last_token;
void compiler_error(struct compile_process* compiler, const char* msg, ...);



/* BEGIN - LAB 2 ---------------------------------*/
// Pega um caracter do arquivo e atualiza a posicao.
char compile_process_next_char(struct lex_process* lex_process) {
    struct compile_process* compiler = lex_process->compiler;
    compiler->pos.col += 1;        // Atualiza a posicao da coluna.
    char c = getc(compiler->cfile.fp);
    if (c == '\n') {
        compiler->pos.line +=1;
        compiler->pos.col =1;
    }

    return c;
}
// Apenas pega um caracter do arquivo.
char compile_process_peek_char(struct lex_process* lex_process) {
    struct compile_process* compiler = lex_process->compiler;
    char c = getc(compiler->cfile.fp);
    ungetc(c, compiler->cfile.fp);
    
    return c;

}
// Adicionar um caracter no arquivo.
void compile_process_push_char(struct lex_process* lex_process, char c) {
    struct compile_process* compiler = lex_process->compiler;
    ungetc(c, compiler->cfile.fp);

}
/* END - LAB 2 ---------------------------------*/


