#include "compiler.h"
#include "helpers/vector.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct vector *node_vector = NULL;
struct vector *node_vector_root = NULL;

void node_set_vector(struct vector *vec, struct vector *root_vec)
{
    node_vector = vec;
    node_vector_root = root_vec;
}

void node_push(struct node *node)
{
    if (!node_vector)
    {
        fprintf(stderr, "Erro crítico: node_vector não inicializado\n");
        exit(EXIT_FAILURE);
    }
    vector_push(node_vector, &node);
}

struct node *node_peek_or_null()
{
    if (!node_vector || vector_count(node_vector) == 0)
    {
        return NULL;
    }
    return *(struct node **)vector_back(node_vector);
}

struct node *node_peek()
{
    struct node *node = node_peek_or_null();
    if (!node)
    {
        fprintf(stderr, "Erro: Nenhum nó disponível\n");
        exit(EXIT_FAILURE);
    }
    return node;
}

struct node *node_pop()
{
    if (!node_vector || vector_count(node_vector) == 0)
    {
        return NULL;
    }
    
    struct node *last_node = vector_back_ptr(node_vector);
    struct node *last_node_root = vector_empty(node_vector) ? NULL : vector_back_ptr_or_null(node_vector_root);
    
    vector_pop(node_vector);
    
    if (last_node == last_node_root)
    {
        vector_pop(node_vector_root);
    }
    
    return last_node;
}

bool node_is_expressionable(struct node *node)
{
    return node->type == NODE_TYPE_EXPRESSION ||
           node->type == NODE_TYPE_EXPRESSION_PARENTHESES ||
           node->type == NODE_TYPE_UNARY ||
           node->type == NODE_TYPE_IDENTIFIER ||
           node->type == NODE_TYPE_NUMBER ||
           node->type == NODE_TYPE_STRING;
}

struct node *node_peek_expressionable_or_null()
{
    struct node *last_node = node_peek_or_null();
    return node_is_expressionable(last_node) ? last_node : NULL;
}

void make_exp_node(struct node *node_left, struct node *node_right, const char *op)
{
    assert(node_left);
    assert(node_right);
    node_create(&(struct node){.type = NODE_TYPE_EXPRESSION, 
                              .exp.left = node_left, 
                              .exp.right = node_right, 
                              .exp.op = op});
}


struct node *node_create(struct node *_node) {
    printf("Debug: Criando nó do tipo %d\n", _node->type); // Debug
    
    struct node *node = malloc(sizeof(struct node));
    memcpy(node, _node, sizeof(struct node));
    node_push(node);
    
    printf("Debug: Nó criado com sucesso (endereço: %p)\n", node); // Debug
    return node;
}