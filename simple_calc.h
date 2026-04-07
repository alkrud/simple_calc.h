#pragma once

#ifndef SIMPLE_CALCULATOR_H_
#define SIMPLE_CALCULATOR_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#ifndef NUM_TYPE
    #define NUM_TYPE double
#endif // NUM_TYPE

NUM_TYPE sc_calculate(const char *text, int len); // -1 for null terminated string

#endif // SIMPLE_CALCULATOR_H_

#ifdef SIMPLE_CALC_IMPLEMENTATION

#ifndef SC_REALLOC
    #define SC_REALLOC(ptr, sz) realloc(ptr, sz)
#endif
#ifndef SC_FREE
    #define SC_FREE(ptr) free(ptr)
#endif

#define sc_list_append(list, item)\
    do {\
        if ((list).count >= (list).capacity) {\
            (list).capacity = (list).capacity < 64 ? 64 : (list).capacity * 2;\
            (list).items = SC_REALLOC((list).items, sizeof(*(list).items) * (list).capacity);\
        }\
        (list).items[(list).count] = item;\
        (list).count += 1;\
    } while (0)

#define sc_list_delete(list) \
    do {\
        SC_FREE((list).items);\
        (list).items = NULL;\
        (list).count = 0;\
        (list).capacity = 0;\
    } while (0)

typedef enum {
    SC_NUM,
    SC_PLUS,
    SC_HYPHEN,
    SC_STAR,
    SC_SLASH,
    SC_CARRET,
    SC_LPAREN,
    SC_RPAREN,
    SC_SYMBOL,
    SC_END,
    SC_ERROR,
    SC_COUNT,
} SC_TokenType;

typedef struct {
    const char *begin;
    const char *end;
    SC_TokenType type;
} SC_Token;

#define TOKEN_LEN(token) ((int)((token).end - (token).begin + 1))

typedef struct {\
    SC_Token *items;
    size_t count;
    size_t capacity;
} SC_TokenList;

typedef struct {
    const char *text;
    size_t text_len;
    size_t current;
} SC_Lexer;

SC_Lexer sc_lexer_new(const char *text, size_t len) 
{
    return (SC_Lexer) {
        .text = text,
        .text_len = len,
        .current = 0,
    };
}

bool sc_is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c == '_');
}

bool sc_is_num(char c)
{
    return c >= '0' && (c) <= '9';
}

bool sc_is_alnum(char c)
{
    return sc_is_alpha(c) || sc_is_num(c);
}

bool sc_is_space(char c)
{
    return c == ' ' || c == '\r' || c == '\t' || c == '\n';
}

char sc_lexer_peek(SC_Lexer *lexer)
{
    return lexer->text[lexer->current];
}

char sc_lexer_consume(SC_Lexer *lexer) 
{
    return lexer->text[lexer->current++];
}

SC_Token sc_tokenize_num(SC_Lexer *lexer)
{
    SC_Token token = {0};
    token.begin = &lexer->text[lexer->current];

    while (lexer->current < lexer->text_len && sc_is_num((sc_lexer_peek(lexer)))) {
        sc_lexer_consume(lexer);
    }

    if (lexer->current < lexer->text_len && sc_lexer_peek(lexer) == '.') {
        sc_lexer_consume(lexer);
        while (lexer->current < lexer->text_len && sc_is_num((sc_lexer_peek(lexer)))) {
            sc_lexer_consume(lexer);
        }
    }

    token.end = &lexer->text[lexer->current - 1];
    token.type = SC_NUM;

    return token;
}

SC_Token sc_tokenize_symbol(SC_Lexer *lexer)
{
    SC_Token token = {0};
    token.begin = &lexer->text[lexer->current];

    while (lexer->current < lexer->text_len && sc_is_alnum(sc_lexer_peek(lexer))) {
        sc_lexer_consume(lexer);
    }

    token.end = &lexer->text[lexer->current - 1];
    token.type = SC_SYMBOL;

    return token;
}

SC_Token sc_tokenize_operator(SC_Lexer *lexer)
{
    SC_Token token = {0};
    token.type = SC_END;

    if (lexer->current >= lexer->text_len) return token;

    token.begin = &lexer->text[lexer->current];

    switch (sc_lexer_peek(lexer)) {
        case '+':
            sc_lexer_consume(lexer);
            token.type = SC_PLUS;
            break;
        case '-':
            sc_lexer_consume(lexer);
            token.type = SC_HYPHEN;
            break;
        case '*':
            sc_lexer_consume(lexer);
            token.type = SC_STAR;
            break;
        case '/':
            sc_lexer_consume(lexer);
            token.type = SC_SLASH;
            break;
        case '^':
            sc_lexer_consume(lexer);
            token.type = SC_CARRET;
            break;
        case '(':
            sc_lexer_consume(lexer);
            token.type = SC_LPAREN;
            break;
        case ')':
            sc_lexer_consume(lexer);
            token.type = SC_RPAREN;
            break;
        default:
            token.type = SC_ERROR;
    }

    token.end = &lexer->text[lexer->current - 1];

    return token;
}

void sc_trim_left(SC_Lexer *lexer)
{
    while (lexer->current < lexer->text_len && sc_is_space(sc_lexer_peek(lexer))) {
        sc_lexer_consume(lexer);
    }
}

SC_Token sc_lexer_next(SC_Lexer *lexer)
{
    sc_trim_left(lexer);

    if (lexer->current >= lexer->text_len) {
        return (SC_Token) {
            .begin = NULL,
            .end = NULL,
            .type = SC_END,
        };
    }

    char c = sc_lexer_peek(lexer);

    if (sc_is_num(c)) {
        return sc_tokenize_num(lexer);
    }
    else if (sc_is_alpha(c)) {
        return sc_tokenize_symbol(lexer);
    }
    else {
        return sc_tokenize_operator(lexer);
    }
}

bool sc_tokenize(SC_Lexer *lexer, SC_TokenList *token_list)
{
    SC_Token token = {0};

    while ((token = sc_lexer_next(lexer)).type != SC_END) {
        sc_list_append(*token_list, token);

        if (token.type == SC_ERROR) return false;
    }

    sc_list_append(*token_list, token);

    return true;
}

typedef struct {
    SC_TokenList *tokens;
    size_t current;
    NUM_TYPE ans;
    bool error;
} SC_Parser;

SC_Parser sc_parser_new(SC_TokenList *token_list)
{
    return (SC_Parser) {
        .tokens = token_list,
        .current = 0,
        .ans = 0,
        .error = false,
    };
}

SC_Token sc_parser_consume(SC_Parser *parser)
{
    return parser->tokens->items[parser->current++];
}

SC_Token sc_parser_peek(SC_Parser *parser)
{
    return parser->tokens->items[parser->current];
}

SC_Token sc_parser_prev(SC_Parser *parser)
{
    return parser->tokens->items[parser->current - 1];
}

typedef enum {
    SC_PREC_NONE,
    SC_PREC_ADDSUB,
    SC_PREC_MULDIV,
    SC_PREC_POW,
    SC_PREC_UNARY,
} SC_Precedence;

typedef NUM_TYPE (*SC_ParseFn)(SC_Parser*);

typedef struct {
    SC_ParseFn prefix;
    SC_ParseFn infix;
    int lbp;
} SC_ParseRule;

NUM_TYPE sc_num(SC_Parser *parser);
NUM_TYPE sc_binary(SC_Parser *parser);
NUM_TYPE sc_unary(SC_Parser *parser);
NUM_TYPE sc_grouping(SC_Parser *parser);
NUM_TYPE sc_identifier(SC_Parser *parser);

static const SC_ParseRule SC_rules[SC_COUNT] = {
    {sc_num, NULL, SC_PREC_NONE},
    {sc_unary, sc_binary, SC_PREC_ADDSUB},
    {sc_unary, sc_binary, SC_PREC_ADDSUB},
    {NULL, sc_binary, SC_PREC_MULDIV},
    {NULL, sc_binary, SC_PREC_MULDIV},
    {NULL, sc_binary, SC_PREC_POW},
    {sc_grouping, NULL, SC_PREC_NONE},
    {NULL, NULL, SC_PREC_NONE},
    {sc_identifier, NULL, SC_PREC_NONE},
    {NULL, NULL, SC_PREC_NONE},
    {NULL, NULL, SC_PREC_NONE},
};

SC_ParseRule sc_get_rule(SC_Token token)
{
    return SC_rules[token.type];
}

NUM_TYPE sc_expression(SC_Parser *parser, SC_Precedence prec)
{
    if (parser->error) return 0;

    SC_Token token = sc_parser_consume(parser);

    if (token.type == SC_END) {
        fprintf(stderr, "ERROR: Expression isn't complete\n");
        parser->error = true;
        return 0;
    }

    SC_ParseRule rule = sc_get_rule(token);

    if (rule.prefix == NULL) {
        fprintf(stderr, "ERROR: token '%.*s' shouldn't be here\n", TOKEN_LEN(token), token.begin);
        parser->error = true;
        return 0;
    }

    NUM_TYPE left = rule.prefix(parser);

    while (parser->current < parser->tokens->count && (int)prec < sc_get_rule(sc_parser_peek(parser)).lbp) {
        token = sc_parser_consume(parser);

        rule = sc_get_rule(token);

        if (rule.infix == NULL) {
            fprintf(stderr, "ERROR: token '%.*s' shouldn't be here\n", TOKEN_LEN(token), token.begin);
            parser->error = true;
            return 0;
        }

        NUM_TYPE right = rule.infix(parser);

        switch (token.type) {
            case SC_PLUS :  left = left + right; break;
            case SC_HYPHEN: left = left - right; break;
            case SC_STAR  : left = left * right; break;
            case SC_SLASH : left = left / right; break;
            case SC_CARRET: left = pow(left, right); break;
            default:
                fprintf(stderr, "ERROR: Unknown token type: '%.*s'\n", TOKEN_LEN(token), token.begin);
                parser->error = true;
                return 0;
        }
    }

    return left;
}

NUM_TYPE sc_num(SC_Parser *parser)
{
    SC_Token token = sc_parser_prev(parser);
    char *endptr;
    char temp[TOKEN_LEN(token) + 1];
    sprintf(temp, "%.*s", TOKEN_LEN(token), token.begin);

    return strtod(temp, &endptr);
}

NUM_TYPE sc_binary(SC_Parser *parser)
{
    SC_Token token = sc_parser_prev(parser);
    SC_ParseRule rule = sc_get_rule(token);

    return sc_expression(parser, (SC_Precedence)(rule.lbp));
}

NUM_TYPE sc_unary(SC_Parser *parser)
{
    SC_Token token = sc_parser_prev(parser);
    NUM_TYPE result = 0;

    switch (token.type) {
        case SC_PLUS:
            result = sc_expression(parser, SC_PREC_UNARY);
            if (result < 0) result *= -1;
            break;
        case SC_HYPHEN:
            result = -sc_expression(parser, SC_PREC_UNARY);
            break;
        case SC_END:
            fprintf(stderr, "ERROR: Expression isn't complete\n");
            parser->error = true;
            break;
        default:
            fprintf(stderr, "Unkown unary operator '%.*s'", TOKEN_LEN(token), token.begin);
            parser->error = true;
            break;
    }

    return result;
}

NUM_TYPE sc_grouping(SC_Parser *parser)
{
    SC_Token token = sc_parser_prev(parser);
    SC_ParseRule rule = sc_get_rule(token);
    NUM_TYPE num = sc_expression(parser, (SC_Precedence)(rule.lbp));

    if (parser->error) return 0;

    if (sc_parser_consume(parser).type != SC_RPAREN) {
        token = sc_parser_prev(parser);

        if (token.type == SC_END) {
            fprintf(stderr, "ERROR: Expresssion isn't complete\n"); 
        }
        else {
            fprintf(stderr, "ERROR: expected ')' but got '%.*s'\n", TOKEN_LEN(token), token.begin); 
        }

        parser->error = true;
        return 0;
    }

    return num;
}

NUM_TYPE sc_identifier(SC_Parser *parser)
{
    (void)(parser);
    return 0;
}

NUM_TYPE sc_parse(SC_TokenList *token_list)
{
    if (token_list->count <= 1) return 0;

    SC_Parser parser = sc_parser_new(token_list);
    NUM_TYPE result = sc_expression(&parser, SC_PREC_NONE);

    if (parser.error) {
        SC_Token token = sc_parser_prev(&parser);
        if (token.type == SC_END) {
            fprintf(stderr, "ERROR: Parsing failed\n");
        }
        else {
            fprintf(stderr, "ERROR: Parsing failed at '%.*s'\n", TOKEN_LEN(token), token.begin);
        }
        return 0;
    }

    return result;
}

NUM_TYPE sc_calculate(const char *text, int len)
{
    if (text == NULL || len == 0) return 0;

    SC_TokenList token_list = {0};
    SC_Lexer lexer = sc_lexer_new(text, len < 0 ? strlen(text) : (size_t)len);

    if (!sc_tokenize(&lexer, &token_list)) {
        fprintf(stderr, "ERROR: Tokenization failed\n");
        sc_list_delete(token_list);
        return 0;
    }

    NUM_TYPE result = sc_parse(&token_list);
    sc_list_delete(token_list);

    return result;
}

#undef SC_REALLOC
#undef SC_FREE
#undef TOKEN_LEN
#undef sc_list_append
#undef sc_list_delete

#endif // SIMPLE_CALC_IMPLEMENTATION
