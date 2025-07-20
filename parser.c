#include "compiler.h"
#include "helpers/vector.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>

// A struct history agora está definida em compiler.h

// Protótipos (APENAS os protótipos que são definidos neste arquivo e não em compiler.h)
void parse_expressionable(struct history* history);
int parse_expressionable_single(struct history* history);
static bool parser_left_op_has_priority(const char* op_left, const char* op_right);
extern struct expressionable_op_precedence_group op_precedence[TOTAL_OPERADOR_GROUPS];

// FUNÇÕES DE TOKEN (NOVAS IMPLEMENTAÇÕES ABAIXO)
static struct token* token_peek_next();
static struct token* token_next();

void parse_variable_function_or_struct_union(struct history* history);
void parse_struct(struct datatype* dtype, struct token* name_token, struct history* history);
static void expect_sym(char c);
static void expect_op(const char* op);


struct history* history_begin(int flags) {
    struct history* history = calloc(1, sizeof(struct history));
    history->flags = flags;
    return history;
}

struct history* history_down(struct history* history, int flags) {
    struct history* new_history = calloc(1, sizeof(struct history));
    memcpy(new_history, history, sizeof(struct history));
    new_history->flags = flags;
    return new_history;
}

// Implementação aprimorada de token_peek_next para lidar com tokens descartáveis (SEM DEBUGs)
static struct token* token_peek_next() {
    struct token* token = vector_peek_no_increment(current_process->token_vec);
    while (token && discard_token(token)) {
        vector_peek(current_process->token_vec); // Avança o ponteiro interno do vetor
        token = vector_peek_no_increment(current_process->token_vec); // Olha para o próximo
    }
    return token;
}

// Implementação aprimorada de token_next para lidar com tokens descartáveis (SEM DEBUGs)
static struct token* token_next() {
    struct token* token = vector_peek(current_process->token_vec); // Pega o token atual e avança o ponteiro
    while (token && discard_token(token)) {
        token = vector_peek(current_process->token_vec); // Pega o próximo (e o consome)
    }

    if(token) {
        current_process->pos = token->pos;
        parser_last_token = token;
    }
    return token;
}


void parse_single_token_to_node() {
    struct token* token = token_next();
    struct node* node = NULL;

    switch (token->type) {
    case TOKEN_TYPE_NUMBER:
        node = node_create(&(struct node){.type=NODE_TYPE_NUMBER, .llnum=token->llnum});
        break;
    case TOKEN_TYPE_IDENTIFIER:
        node = node_create(&(struct node){.type=NODE_TYPE_IDENTIFIER, .sval=token->sval});
        break;
    case TOKEN_TYPE_STRING:
        node = node_create(&(struct node){.type=NODE_TYPE_STRING, .sval=token->sval});
        break;
    default:
        compiler_error(current_process, "Esse token nao pode ser convertido para node!\n");
    }
}

void parse_expressionable_for_op(struct history* history, const char* op) {
    parse_expressionable(history);
}

void parser_node_shift_children_left(struct node* node) {
    assert(node->type == NODE_TYPE_EXPRESSION);
    assert(node->exp.right->type == NODE_TYPE_EXPRESSION);

    const char* right_op = node->exp.right->exp.op;
    struct node* new_exp_left_node  = node->exp.left;
    struct node* new_exp_right_node = node->exp.right->exp.left;
    make_exp_node(new_exp_left_node, new_exp_right_node, node->exp.op);

    struct node* new_left_operand = node_pop();
    struct node* new_right_operand = node->exp.right->exp.right;
    node->exp.left = new_left_operand;
    node->exp.right = new_right_operand;
    node->exp.op = right_op;
}

void parser_reorder_expression(struct node** node_out) {
    struct node* node = *node_out;
    if (node->type != NODE_TYPE_EXPRESSION) return;
    if (node->exp.left->type != NODE_TYPE_EXPRESSION && node->exp.right && node->exp.right->type != NODE_TYPE_EXPRESSION) return;

    if (node->exp.left->type != NODE_TYPE_EXPRESSION && node->exp.right && node->exp.right->type == NODE_TYPE_EXPRESSION) {
        const char* right_op = node->exp.right->exp.op;
        if (parser_left_op_has_priority(node->exp.op, right_op)) {
            parser_node_shift_children_left(node);
            parser_reorder_expression(&node->exp.left);
            parser_reorder_expression(&node->exp.right);
        }
    }
}

static int parser_get_precedence_for_operator(const char* op, struct expressionable_op_precedence_group** group_out) {
    *group_out = NULL;
    for (int i = 0; i < TOTAL_OPERADOR_GROUPS; i++) {
        for (int j = 0; op_precedence[i].operators[j]; j++) {
            if (S_EQ(op, op_precedence[i].operators[j])) {
                *group_out = &op_precedence[i];
                return i;
            }
        }
    }
    return -1;
}

static bool parser_left_op_has_priority(const char* op_left, const char* op_right) {
    struct expressionable_op_precedence_group* group_left = NULL;
    struct expressionable_op_precedence_group* group_right = NULL;
    if (S_EQ(op_left, op_right)) return false;
    int precedence_left = parser_get_precedence_for_operator(op_left, &group_left);
    int precedence_right = parser_get_precedence_for_operator(op_right, &group_right);
    if (group_left->associativity == ASSOCIATIVITY_RIGHT_TO_LEFT) return false;
    return precedence_left <= precedence_right;
}

void parse_exp_normal(struct history* history) {
    struct token* op_token = token_peek_next();
    const char* op = op_token->sval;
    struct node* node_left = node_peek_expressionable_or_null();
    if (!node_left) return;

    token_next();
    node_pop();
    node_left->flags |= NODE_FLAG_INSIDE_EXPRESSION;
    parse_expressionable_for_op(history_down(history, history->flags), op);
    struct node* node_right = node_pop();
    node_right->flags |= NODE_FLAG_INSIDE_EXPRESSION;

    make_exp_node(node_left, node_right, op);
    struct node* exp_node = node_pop();
    parser_reorder_expression(&exp_node);
    node_push(exp_node);
}

int parse_exp(struct history* history) {
    parse_exp_normal(history);
    return 0;
}

void parse_identifier(struct history* history) {
    assert(token_peek_next()->type == TOKEN_TYPE_IDENTIFIER);
    parse_single_token_to_node();
}

static bool keyword_is_datatype(const char* val) {
    return S_EQ(val, "void") || S_EQ(val, "char") || S_EQ(val, "int") || S_EQ(val, "short") ||
           S_EQ(val, "float") || S_EQ(val, "double") || S_EQ(val, "long") || S_EQ(val, "struct") ||
           S_EQ(val, "union");
}

static bool is_keyword_variable_modifier(const char* val) {
    return S_EQ(val, "unsigned") || S_EQ(val, "signed") || S_EQ(val, "static") ||
           S_EQ(val, "const") || S_EQ(val, "extern") || S_EQ(val, "__ignore_typecheck__");
}

bool token_is_primitive_keyword(struct token* token) {
    if (!token || token->type != TOKEN_TYPE_KEYWORD) return false;
    return S_EQ(token->sval, "void") || S_EQ(token->sval, "char") || S_EQ(token->sval, "short") ||
           S_EQ(token->sval, "int") || S_EQ(token->sval, "long") || S_EQ(token->sval, "float") ||
           S_EQ(token->sval, "double");
}

void parser_get_datatype_tokens(struct token** datatype_token, struct token** datatype_secundary_token) {
    *datatype_token = token_next();
    struct token* next_token = token_peek_next();
    if (token_is_primitive_keyword(next_token)) {
        *datatype_secundary_token = next_token;
        token_next();
    }
}

int parser_datatype_expected_for_type_string(const char* str) {
    if (S_EQ(str, "union")) return DATATYPE_EXPECT_UNION;
    if (S_EQ(str, "struct")) return DATATYPE_EXPECT_STRUCT;
    return DATATYPE_EXPECT_PRIMITIVE;
}

int parser_get_random_type_index() {
    static int initialized = 0;
    static int last = 0;
    if (!initialized) {
        srand((unsigned int)time(NULL));
        initialized = 1;
    }
    int r;
    do { r = rand(); } while (r <= last);
    last = r;
    return r;
}

struct token* parser_build_random_type_name() {
    char tmp_name[25];
    sprintf(tmp_name, "customtypename_%i", parser_get_random_type_index());
    char* sval = malloc(strlen(tmp_name) + 1);
    strcpy(sval, tmp_name);
    struct token* token = calloc(1, sizeof(struct token));
    token->type = TOKEN_TYPE_IDENTIFIER;
    token->sval = sval;
    return token;
}


static bool token_next_is_operator(const char* op) {
    struct token* token = token_peek_next();
    return token_is_operator(token, op);
}

int parser_get_pointer_depth() {
    int depth = 0;
    while (token_next_is_operator("*")) {
        depth++;
        token_next();
    }
    return depth;
}

bool parser_datatype_is_secondary_allowed_for_type(const char* type) {
    return S_EQ(type, "long") || S_EQ(type, "short") || S_EQ(type, "double") || S_EQ(type, "float");
}

void parser_datatype_init_type_and_size_for_primitive(struct token* datatype_token, struct token* datatype_secondary_token, struct datatype* datatype_out);

void parser_datatype_adjust_size_for_secondary(struct datatype* datatype, struct token* datatype_secondary_token) {
    if (!datatype_secondary_token) return;
    struct datatype* secondary_data_type = calloc(1, sizeof(struct datatype));
    parser_datatype_init_type_and_size_for_primitive(datatype_secondary_token, NULL, secondary_data_type);
    datatype->size += secondary_data_type->size;
    datatype->datatype_secondary = secondary_data_type;
    datatype->flags |= DATATYPE_FLAG_IS_SECONDARY;
}

void parser_datatype_init_type_and_size_for_primitive(struct token* datatype_token, struct token* datatype_secondary_token, struct datatype* datatype_out) {
    if (!parser_datatype_is_secondary_allowed_for_type(datatype_token->sval) && datatype_secondary_token) {
        compiler_error(current_process, "Voce utilizou um datatype secundario invalido!\n");
    }
    if (S_EQ(datatype_token->sval, "void")) { datatype_out->type = DATATYPE_VOID; datatype_out->size = 0; }
    else if (S_EQ(datatype_token->sval, "char")) { datatype_out->type = DATATYPE_CHAR; datatype_out->size = 1; }
    else if (S_EQ(datatype_token->sval, "short")) { datatype_out->type = DATATYPE_SHORT; datatype_out->size = 2; }
    else if (S_EQ(datatype_token->sval, "int")) { datatype_out->type = DATATYPE_INTEGER; datatype_out->size = 4; }
    else if (S_EQ(datatype_token->sval, "long")) { datatype_out->type = DATATYPE_LONG; datatype_out->size = 8; }
    else if (S_EQ(datatype_token->sval, "float")) { datatype_out->type = DATATYPE_FLOAT; datatype_out->size = 4; }
    else if (S_EQ(datatype_token->sval, "double")) { datatype_out->type = DATATYPE_DOUBLE; datatype_out->size = 8; }
    else compiler_error(current_process, "BUG: Datatype invalido!\n");
    parser_datatype_adjust_size_for_secondary(datatype_out, datatype_secondary_token);
}

void parser_datatype_init_type_and_size(struct token* datatype_token, struct token* datatype_secondary_token, struct datatype* datatype_out, int pointer_depth, int expected_type) {
    if (expected_type != DATATYPE_EXPECT_PRIMITIVE && datatype_secondary_token) {
        compiler_error(current_process, "Voce utilizou um datatype secundario invalido para struct/union!\n");
    }
    switch (expected_type) {
        case DATATYPE_EXPECT_PRIMITIVE:
            parser_datatype_init_type_and_size_for_primitive(datatype_token, datatype_secondary_token, datatype_out);
            break;
        case DATATYPE_EXPECT_STRUCT:
        case DATATYPE_EXPECT_UNION:
            datatype_out->type = (expected_type == DATATYPE_EXPECT_STRUCT) ? DATATYPE_STRUCT : DATATYPE_UNION;
            datatype_out->size = 0;
            break;
        default: compiler_error(current_process, "BUG: Erro desconhecido!\n");
    }
}

void parser_datatype_init(struct token* datatype_token, struct token* datatype_secondary_token, struct datatype* datatype_out, int pointer_depth, int expected_type) {
    parser_datatype_init_type_and_size(datatype_token, datatype_secondary_token, datatype_out, pointer_depth, expected_type);
    datatype_out->type_str = datatype_token->sval;
    datatype_out->pointer_depth = pointer_depth;
}

// MODIFICADO: Agora retorna struct token*
struct token* parse_datatype_type(struct datatype* dtype) {
    struct token* datatype_token = NULL;
    struct token* datatype_secundary_token = NULL;
    parser_get_datatype_tokens(&datatype_token, &datatype_secundary_token);
    int expected_type = parser_datatype_expected_for_type_string(datatype_token->sval);

    struct token* name_token_for_struct_union = NULL; // NOVO: Armazena o token do nome da struct/union

    if (S_EQ(datatype_token->sval, "union") || S_EQ(datatype_token->sval, "struct")) {
        if (token_peek_next()->type == TOKEN_TYPE_IDENTIFIER) {
            name_token_for_struct_union = token_next(); // Consome o identificador (nome da struct/union)
            dtype->type_str = name_token_for_struct_union->sval; // Define o nome da struct/union aqui
        } else {
            if(token_peek_next()->cval != '{')
                compiler_error(current_process, "Nome da struct/union esperado ou '{' para struct anonima.\n");
            dtype->flags |= DATATYPE_FLAG_IS_STRUCT_UNION_NO_NAME;
        }
    }
    int pointer_depth = parser_get_pointer_depth();
    parser_datatype_init(datatype_token, datatype_secundary_token, dtype, pointer_depth, expected_type);

    // Se name_token_for_struct_union não é NULL, significa que é uma struct/union nomeada
    // e o dtype->type_str já foi atualizado para o nome dela.
    // Caso contrário (tipo primitivo ou struct/union sem nome), dtype->type_str já foi definido por parser_datatype_init.
    return name_token_for_struct_union; // Retorna o token do nome da struct/union, ou NULL para outros tipos
}

void parse_datatype_modifiers(struct datatype* dtype) {
    struct token* token = token_peek_next();
    while (token && token->type == TOKEN_TYPE_KEYWORD) {
        if (!is_keyword_variable_modifier(token->sval)) break;
        if (S_EQ(token->sval, "signed")) dtype->flags |= DATATYPE_FLAG_IS_SIGNED;
        else if (S_EQ(token->sval, "unsigned")) dtype->flags &= ~DATATYPE_FLAG_IS_SIGNED;
        else if (S_EQ(token->sval, "static")) dtype->flags |= DATATYPE_FLAG_IS_STATIC;
        else if (S_EQ(token->sval, "const")) dtype->flags |= DATATYPE_FLAG_IS_CONST;
        else if (S_EQ(token->sval, "extern")) dtype->flags |= DATATYPE_FLAG_IS_EXTERN;
        else if (S_EQ(token->sval, "__ignore_typecheck__")) dtype->flags |= DATATYPE_FLAG_IS_IGNORE_TYPE_CHECKING;
        token_next();
        token = token_peek_next();
    }
}


void parse_datatype(struct datatype* dtype) {
    memset(dtype, 0, sizeof(struct datatype));
    dtype->flags |= DATATYPE_FLAG_IS_SIGNED;
    parse_datatype_modifiers(dtype);
    parse_datatype_type(dtype);
    parse_datatype_modifiers(dtype);
}

// CORREÇÃO FINAL: Função corrigida para não fazer pop de uma pilha vazia.
void parse_expressionable_root(struct history* history) {
    parse_expressionable(history);
    struct node* result_node = node_peek_or_null();
    if (result_node) {
        result_node = node_pop();
        node_push(result_node);
    }
}


void make_variable_node(struct datatype* dtype, struct token* name_token, struct node* value_node) {
    const char* name_str = name_token ? name_token->sval : NULL;
    node_create(&(struct node){.type = NODE_TYPE_VARIABLE, .var.name = name_str, .var.type = *dtype, .var.val = value_node});
}

void make_variable_node_and_register(struct history* history, struct datatype* dtype, struct token* name_token, struct node* value_node) {
    make_variable_node(dtype, name_token, value_node);
    struct node* var_node = node_pop();
    symresolver_build_for_node(current_process, var_node);
    scope_push(current_process, var_node, var_node->var.type.size); // Passa o tamanho da variável para o escopo
    node_push(var_node);
}

static void expect_sym(char c) {
    struct token* next_token = token_next();
    if (!next_token) {
        compiler_error(current_process, "Esperado o simbolo %c, mas encontrou EOF (fim de arquivo).\n", c);
    }
    if (next_token->type != TOKEN_TYPE_SYMBOL || next_token->cval != c) {
        compiler_error(current_process, "Esperado o simbolo %c, mas outra coisa foi fornecida: '%s' (tipo %d, valor char '%c').\n", c, next_token->sval ? next_token->sval : "N/A", next_token->type, next_token->cval);
    }
    // printf("DEBUG: Consumiu simbolo '%c'\n", c); // Removido
}

static void expect_op(const char* op) {
    struct token* next_token = token_next();
    if (!next_token) {
        compiler_error(current_process, "Esperado o operador %s, mas encontrou EOF (fim de arquivo).\n", op);
    }
    if (next_token->type != TOKEN_TYPE_OPERATOR || !S_EQ(next_token->sval, op)) {
        compiler_error(current_process, "Esperado o operador %s, mas foi fornecido '%s' (tipo %d).\n", op, next_token->sval ? next_token->sval : "N/A", next_token->type);
    }
    // printf("DEBUG: Consumiu operador '%s'\n", op); // Removido
}

void parse_array_brackets(struct history* history, struct datatype* dtype) {
    if (!dtype->array_brackets) {
        dtype->array_brackets = vector_create(sizeof(struct node*));
    }
    while (token_next_is_operator("[")) {
        expect_op("[");
        parse_expressionable_root(history);
        struct node* bracket_node = node_pop();
        node_create(&(struct node){.type = NODE_TYPE_BRACKET, .exp.left = bracket_node});
        struct node* final_bracket_node = node_pop();
        vector_push(dtype->array_brackets, &final_bracket_node);
        // printf("DEBUG: Prestes a esperar ']' para array\n"); // Removido
        expect_op("]");
    }
}

void parse_variable(struct datatype* dtype, struct token* name_token, struct history* history) {
    dtype->array_brackets = NULL;
    if (token_next_is_operator("[")) {
        parse_array_brackets(history, dtype);
    }
    struct node* value_node = NULL;
    if (token_next_is_operator("=")) {
        token_next();
        parse_expressionable_root(history);
        value_node = node_pop();
    }
    make_variable_node_and_register(history, dtype, name_token, value_node);
}

void parse_statement(struct history* history);

void parse_body(struct history* history) {
    expect_sym('{');
    struct vector* statements = vector_create(sizeof(struct node*));
    scope_new(current_process, 0);
    while (token_peek_next()->type != TOKEN_TYPE_SYMBOL || token_peek_next()->cval != '}') {
        parse_statement(history);
        struct node* stmt_node = node_pop();
        vector_push(statements, &stmt_node);
    }
    expect_sym('}');
    scope_finish(current_process);
    node_create(&(struct node){.type=NODE_TYPE_BODY, .body.statements=statements});
}

void parse_struct_body(struct history* history) {
    expect_sym('{');
    struct vector* statements = vector_create(sizeof(struct node*));
    scope_new(current_process, 0);
    while (token_peek_next()->type != TOKEN_TYPE_SYMBOL || token_peek_next()->cval != '}') {
        parse_variable_function_or_struct_union(history);
        struct node* stmt_node = node_pop();
        vector_push(statements, &stmt_node);
    }
    expect_sym('}');
    scope_finish(current_process);
    node_create(&(struct node){.type=NODE_TYPE_BODY, .body.statements=statements});
}

void parse_struct(struct datatype* dtype, struct token* name_token, struct history* history) {
    node_create(&(struct node){.type=NODE_TYPE_STRUCT, ._struct.name=name_token->sval});
    struct node* struct_node = node_pop();
    symresolver_build_for_node(current_process, struct_node);

    parse_struct_body(history);
    struct_node->_struct.body_n = node_pop();

    node_push(struct_node);
    expect_sym(';');
}

void parse_function(struct datatype* dtype, struct token* name_token, struct history* history) {
    // A lista de argumentos seria parseada aqui se não estivesse vazia.
    // O expect_sym(')') já consome o ')' que fecha a lista de argumentos.
    expect_sym(')'); // Fecha a lista de parâmetros vazia

    parse_body(history); // Parseia o corpo da função {...}
    struct node* body_node = node_pop();

    node_create(&(struct node){
        .type=NODE_TYPE_FUNCTION,
        .func.name=name_token->sval,
        .func.rtype=*dtype,
        .func.body_n=body_node
    });

    struct node* func_node = node_pop();
    symresolver_build_for_node(current_process, func_node);
    node_push(func_node);
    // Funções em C não são seguidas por ';' após o corpo, então não há expect_sym(';'); aqui.
}

void parse_if_statement(struct history* history) {
    expect_op("(");
    parse_expressionable_root(history);
    struct node* cond_node = node_pop();
    expect_sym(')');

    parse_body(history);
    struct node* body_node = node_pop();

    node_create(&(struct node){.type=NODE_TYPE_STATEMENT_IF, .if_stmt.cond_node=cond_node, .if_stmt.body_node=body_node});
}

// NOVO: Implementação para a nova sintaxe de if/else (if Expr: Stmt: Stmt;)
void parse_new_if_else_statement(struct history* history) {
    // A expressão da condição (ex: A>B)
    parse_expressionable_root(history); // Parseia a expressão sem parênteses
    struct node* cond_node = node_pop();

    expect_sym(':'); // Espera o primeiro ':' após a condição
    // printf("DEBUG: Consumiu ':' apos condicao do IF\n"); // Removido

    // O Statement do bloco TRUE
    // Aqui, o "corpo" é uma única statement, não um bloco entre chaves.
    parse_statement(history); // Chama parse_statement para a declaração do THEN
    struct node* true_body_node = node_pop();

    struct node* else_node = NULL;
    // Verifica se há um 'else' na nova sintaxe (indicado por outro ':')
    if (token_peek_next()->type == TOKEN_TYPE_SYMBOL && token_peek_next()->cval == ':') {
        token_next(); // Consome o segundo ':'
        // printf("DEBUG: Consumiu ':' para o ELSE na nova sintaxe\n"); // Removido

        // O Statement do bloco FALSE (else)
        parse_statement(history); // Chama parse_statement para a declaração do ELSE
        struct node* false_body_node = node_pop();
        
        // Cria o nó para o else
        else_node = node_create(&(struct node){
            .type=NODE_TYPE_STATEMENT_ELSE,
            .if_stmt.body_node=false_body_node // Reutiliza if_stmt para o corpo do else
        });
    }

    // Cria o nó principal do IF
    node_create(&(struct node){
        .type=NODE_TYPE_STATEMENT_IF,
        .if_stmt.cond_node=cond_node,
        .if_stmt.body_node=true_body_node,
        .if_stmt.next=else_node // O nó else (se existir) é ligado ao 'next' do if_stmt
    });

    expect_sym(';'); // A nova sintaxe termina com ';'
    // printf("DEBUG: Consumiu ';' no final do IF/ELSE\n"); // Removido
}


void parse_return_statement(struct history* history) {
    struct node* exp_node = NULL;

    if (token_peek_next()->cval != ';') {
        parse_expressionable_root(history);
        exp_node = node_pop();
    }

    expect_sym(';');
    node_create(&(struct node){.type=NODE_TYPE_STATEMENT_RETURN, .ret_stmt.exp_node=exp_node});
}


void parse_statement(struct history* history) {
    struct token* token = token_peek_next();
    if (token_is_keyword(token, "if")) {
        token_next(); // Consome o 'if'
        // Agora, precisamos verificar se é a sintaxe tradicional (com '(')
        // ou a nova sintaxe (com expressão direta, sem parênteses, seguida por ':')
        struct token* next_token_after_if = token_peek_next();
        if (next_token_after_if->type == TOKEN_TYPE_OPERATOR && next_token_after_if->cval == '(') {
            // Sintaxe tradicional: if (...) { ... } [else { ... }]
            token_next(); // Consome o '('
            parse_if_statement(history); // Chama a função existente
        } else {
            // NOVA SINTAXE: if Expression: Statement: Statement;
            parse_new_if_else_statement(history); // Chama a nova função
        }
        return;
    }
    if (token_is_keyword(token, "return")) {
        token_next();
        parse_return_statement(history);
        return;
    }

    // Adicionado tratamento para nova declaração de variável dentro de blocos de código
    if (token->type == TOKEN_TYPE_DOLLAR_SIGN) {
        token_next(); // Consome o '$'
        parse_new_variable_declaration(history); // Chama a nova função
        return;
    }

    parse_variable_function_or_struct_union(history);
}

void parse_variable_function_or_struct_union(struct history* history) {
    struct datatype dtype;
    parse_datatype(&dtype);

    struct token* name_token = NULL;
    struct token* token_after_type = token_peek_next(); // O token que vem logo após o tipo de dado
    
    bool declaration_handled_internally = false;

    if (dtype.flags & DATATYPE_FLAG_IS_STRUCT_UNION_NO_NAME) {
        parse_struct(&dtype, parser_build_random_type_name(), history);
        declaration_handled_internally = true;
    } else {
        // Consumir o name_token AQUI, antes das verificações de função/struct/nova_struct.
        name_token = token_next();
        if (name_token->type != TOKEN_TYPE_IDENTIFIER) {
            compiler_error(current_process, "Declaracao invalida, nome esperado. Recebido tipo %d, valor '%s'.\n", name_token->type, name_token->sval ? name_token->sval : "NULL");
        }

        struct token* next_token_after_name = token_peek_next(); // Agora, peek no token APÓS o nome

        if (next_token_after_name->type == TOKEN_TYPE_SYMBOL && next_token_after_name->cval == '(') {
            // É uma declaração de função (ex: int main())
            token_next(); // Consome o '('
            parse_function(&dtype, name_token, history);
            declaration_handled_internally = true;
        } else if ((dtype.type == DATATYPE_STRUCT || dtype.type == DATATYPE_UNION) && next_token_after_name->cval == '{') {
            // É uma definição de struct/union com corpo (sintaxe tradicional) (ex: struct Ponto { ... })
            parse_struct(&dtype, name_token, history); // name_token já é o nome correto
            declaration_handled_internally = true;
        } else if (dtype.type == DATATYPE_STRUCT && token_is_operator(next_token_after_name, "=")) {
            // É a nova sintaxe de struct (ex: struct Retangulo = int a, int b;)
            token_next(); // Consome o operador '='
            parse_new_struct_definition(&dtype, name_token, history);
            declaration_handled_internally = true;
        } else {
            // Se chegou aqui, é uma declaração de variável simples ou uma lista de variáveis.
            parse_variable(&dtype, name_token, history); // Parseia a variável

            // Lidar com lista de variáveis (ex: int a, b, c;)
            if (token_is_operator(token_peek_next(), ",")) {
                struct vector* var_list = vector_create(sizeof(struct node*));
                struct node* var_node = node_pop();
                vector_push(var_list, &var_node);
                while (token_is_operator(token_peek_next(), ",")) {
                    token_next(); // Consome a vírgula
                    name_token = token_next(); // Pega o próximo identificador
                    if (name_token->type != TOKEN_TYPE_IDENTIFIER) {
                        compiler_error(current_process, "Esperado um identificador apos ',' em lista de variaveis.\n");
                    }
                    parse_variable(&dtype, name_token, history);
                    var_node = node_pop();
                    vector_push(var_list, &var_node);
                }
                node_create(&(struct node){.type = NODE_TYPE_VARIABLE_LIST, .var_list.list = var_list});
            }
        }
    }

    // Apenas espere o ';' se a declaração NÃO foi completamente tratada (e terminada com ';')
    if (!declaration_handled_internally) {
        expect_sym(';');
    }
}

// Função para analisar a nova declaração de variável ($A=int;)
void parse_new_variable_declaration(struct history* history) {
    // Espera o identificador (nome da variável)
    struct token* name_token = token_next();
    if (name_token->type != TOKEN_TYPE_IDENTIFIER) {
        compiler_error(current_process, "Esperado um identificador apos o '$' para a declaracao de variavel.\n");
    }

    // Espera o operador '='
    expect_op("=");

    // Espera o token do tipo de dado (int, float, etc.)
    struct token* datatype_token = token_next();
    if (!token_is_primitive_keyword(datatype_token)) {
         compiler_error(current_process, "Esperado um tipo de dado primitivo apos '=' na declaracao de variavel.\n");
    }

    // Cria a estrutura de tipo de dado
    struct datatype dtype;
    memset(&dtype, 0, sizeof(struct datatype)); // Inicializa a struct com zeros
    dtype.flags |= DATATYPE_FLAG_IS_SIGNED; // Assumimos que é signed por padrão

    // Inicializa o tipo de dado com base no token encontrado
    parser_datatype_init_type_and_size_for_primitive(datatype_token, NULL, &dtype);
    dtype.type_str = datatype_token->sval; // Define o nome do tipo (ex: "int")
    dtype.pointer_depth = 0; // Por enquanto, não estamos lidando com ponteiros nesta nova sintaxe

    // Por enquanto, não estamos lidando com inicialização de valor nesta nova sintaxe, então val = NULL
    make_variable_node_and_register(history, &dtype, name_token, NULL);

    // Espera o ponto e vírgula
    expect_sym(';');
}

// NOVO: Implementação para a nova sintaxe de declaração de struct (struct Name = member1, member2;)
void parse_new_struct_definition(struct datatype* dtype, struct token* name_token, struct history* history) {
    node_create(&(struct node){.type=NODE_TYPE_STRUCT, ._struct.name=name_token->sval});
    struct node* struct_node = node_pop();
    symresolver_build_for_node(current_process, struct_node);

    struct vector* members = vector_create(sizeof(struct node*));
    scope_new(current_process, 0); // Novo escopo para os membros da struct

    // Loop para ler os membros separados por vírgula até encontrar o ';'
    while (token_peek_next()->cval != ';') {
        // Cada membro é parseado como uma declaração de variável
        struct datatype member_dtype;
        parse_datatype(&member_dtype); // Parseia o tipo do membro

        struct token* member_name_token = token_next(); // Pega o nome do membro
        if (member_name_token->type != TOKEN_TYPE_IDENTIFIER) {
            compiler_error(current_process, "Esperado um identificador para o membro da struct '%s'.\n", name_token->sval);
        }

        // Cria o nó da variável para o membro e registra
        make_variable_node_and_register(history, &member_dtype, member_name_token, NULL);
        struct node* member_node = node_pop();
        vector_push(members, &member_node);

        // Se o próximo token é uma vírgula, consome e continua o loop
        if (token_is_operator(token_peek_next(), ",")) {
            token_next(); // Consome a vírgula
            if (token_peek_next()->cval == ';') { // Evita vírgula pendurada no final
                 compiler_error(current_process, "Virgula inesperada antes do ';'.\n");
            }
        } else if (token_peek_next()->cval != ';') {
            // Se não é vírgula nem ponto e vírgula, algo está errado
            compiler_error(current_process, "Esperado ',' ou ';' apos declaracao de membro da struct '%s'.\n", name_token->sval);
        }
    }
    
    // Cria o nó do corpo para os membros da struct
    node_create(&(struct node){.type=NODE_TYPE_BODY, .body.statements=members});
    struct_node->_struct.body_n = node_pop();

    scope_finish(current_process); // Finaliza o escopo dos membros

    node_push(struct_node); // Empurra o nó da struct de volta para a pilha
    expect_sym(';'); // Espera o ';' final da declaração da struct
}


int parse_expressionable_single(struct history* history) {
    struct token* token = token_peek_next();
    if (!token) return -1;
    history->flags |= NODE_FLAG_INSIDE_EXPRESSION;
    int res = -1;
    switch (token->type) {
    case TOKEN_TYPE_NUMBER: parse_single_token_to_node(); res = 0; break;
    case TOKEN_TYPE_IDENTIFIER: parse_identifier(history); res = 0; break;
    case TOKEN_TYPE_OPERATOR: parse_exp(history); res = 0; break;
    default: break;
    }
    return res;
}

void parse_expressionable(struct history* history){
    while (parse_expressionable_single(history) == 0) {}
}

int parse_next() {
    struct token* token = token_peek_next();
    if (!token) return -1;

    // Adicionar tratamento para a nova declaração de variável
    if (token->type == TOKEN_TYPE_DOLLAR_SIGN) {
        token_next(); // Consome o '$'
        parse_new_variable_declaration(history_begin(0));
        return 0;
    }

    parse_variable_function_or_struct_union(history_begin(0));
    return 0;
}

int parse(struct compile_process* process) {
    current_process = process;
    parser_last_token = NULL;
    node_set_vector(process->node_vec, process->node_tree_vec);
    scope_create_root(process);
    symresolver_initialize(process);

    struct node* node = NULL;
    vector_set_peek_pointer(process->token_vec, 0); // Garante que o ponteiro de leitura está no início.

    while (token_peek_next()) { // Continua enquanto houver tokens não descartáveis
        parse_next();
        node = node_pop();
        if(node) {
            vector_push(process->node_tree_vec, &node);
        }
    }

    return PARSE_ALL_OK;
}