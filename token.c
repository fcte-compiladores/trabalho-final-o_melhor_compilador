#include "compiler.h"
#include "helpers/vector.h"

// declarações
extern struct compile_process *current_process;
extern struct token *parser_last_token;

bool token_is_operator(struct token* token, const char* val) {
    return token && token->type == TOKEN_TYPE_OPERATOR && S_EQ(token->sval, val);
}

struct token* token_peek_next() {
    if (!current_process || !current_process->token_vec) return NULL;
    
    // Get current position
    size_t current_index = 0;
    if (parser_last_token) {
        current_index = ((size_t)(parser_last_token - (struct token*)current_process->token_vec->data)) + 1;
    }
    
    // Check bounds
    if (current_index >= vector_count(current_process->token_vec)) {
        return NULL;
    }
    
    return vector_at(current_process->token_vec, current_index);
}

struct token* token_next() {
    struct token* next_token = token_peek_next();
    if (next_token) {
        parser_last_token = next_token;
    }
    return next_token;
}

bool token_is_keyword(struct token *token, const char *value) {
    return token && token->type == TOKEN_TYPE_KEYWORD && S_EQ(token->sval, value);
}

bool token_is_symbol(struct token *token, char value) {
    return token && token->type == TOKEN_TYPE_SYMBOL && token->cval == value;
}

bool discard_token(struct token* token) {
    return (token->type == TOKEN_TYPE_NEWLINE) ||
           (token->type == TOKEN_TYPE_COMMENT) ||
           token_is_symbol(token, '\\');
}