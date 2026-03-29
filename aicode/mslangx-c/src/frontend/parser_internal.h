#ifndef MS_PARSER_INTERNAL_H_
#define MS_PARSER_INTERNAL_H_

#include "ms/buffer.h"
#include "ms/parser.h"

enum {
  MS_PREC_ASSIGNMENT = 1,
  MS_PREC_LOGICAL_OR = 2,
  MS_PREC_LOGICAL_AND = 3,
  MS_PREC_EQUALITY = 4,
  MS_PREC_COMPARISON = 5,
  MS_PREC_ADDITIVE = 6,
  MS_PREC_MULTIPLICATIVE = 7,
  MS_PREC_UNARY = 8,
  MS_PREC_POSTFIX = 9
};

MsToken ms_parser_make_eof_token(const MsParser *parser);
void ms_parser_advance(MsParser *parser);
int ms_parser_check(const MsParser *parser, MsTokenKind kind);
int ms_parser_match(MsParser *parser, MsTokenKind kind);
void ms_parser_skip_newlines(MsParser *parser);
void ms_parser_append_diagnostic(MsParser *parser,
                                 const char *code,
                                 const char *message,
                                 int line,
                                 int column,
                                 int length);
void ms_parser_error_at_token(MsParser *parser,
                              MsToken token,
                              const char *code,
                              const char *message);
void ms_parser_error_at_node(MsParser *parser,
                             const MsAstNode *node,
                             const char *code,
                             const char *message);
int ms_parser_expect(MsParser *parser,
                     MsTokenKind kind,
                     const char *code,
                     const char *message,
                     MsToken *out_token);
MsAstNode *ms_parser_alloc_node(MsParser *parser,
                                MsAstKind kind,
                                int line,
                                int column,
                                int end_column);
int ms_parser_copy_node_array(MsParser *parser,
                              const MsBuffer *buffer,
                              MsAstNodeArray *out_array);
int ms_parser_copy_token_array(MsParser *parser,
                               const MsBuffer *buffer,
                               MsTokenArray *out_array);
int ms_parser_copy_map_array(MsParser *parser,
                             const MsBuffer *buffer,
                             MsAstMapEntryArray *out_array);
int ms_parser_is_assignable(const MsAstNode *node);
int ms_parser_infix_precedence(MsTokenKind kind);
MsAstNode *ms_parser_make_literal_node(MsParser *parser, MsToken token);
MsAstNode *ms_parser_make_variable_node(MsParser *parser, MsToken name);
MsAstNode *ms_parser_make_self_node(MsParser *parser, MsToken keyword);
MsAstNode *ms_parser_make_super_node(MsParser *parser,
                                     MsToken keyword,
                                     MsToken method);
MsAstNode *ms_parser_make_unary_node(MsParser *parser,
                                     MsToken op,
                                     MsAstNode *operand);
MsAstNode *ms_parser_make_binary_node(MsParser *parser,
                                      MsAstNode *left,
                                      MsToken op,
                                      MsAstNode *right);
MsAstNode *ms_parser_make_logical_node(MsParser *parser,
                                       MsAstNode *left,
                                       MsToken op,
                                       MsAstNode *right);
MsAstNode *ms_parser_make_assign_node(MsParser *parser,
                                      MsAstNode *target,
                                      MsAstNode *value);
MsAstNode *ms_parser_make_property_node(MsParser *parser,
                                        MsAstNode *object,
                                        MsToken name);
MsAstNode *ms_parser_make_index_node(MsParser *parser,
                                     MsAstNode *object,
                                     MsAstNode *index_expr,
                                     MsToken close_bracket);
MsAstNode *ms_parser_make_call_node(MsParser *parser,
                                    MsAstNode *callee,
                                    const MsBuffer *args,
                                    MsToken close_paren);
MsAstNode *ms_parser_make_map_node(MsParser *parser,
                                   const MsBuffer *entries,
                                   int line,
                                   int column,
                                   int end_column);
MsAstNode *ms_parser_make_sequence_node(MsParser *parser,
                                        MsAstKind kind,
                                        const MsBuffer *elements,
                                        int line,
                                        int column,
                                        int end_column);
MsAstNode *ms_parser_make_function_node(MsParser *parser,
                                        MsToken fn_token,
                                        const MsBuffer *params,
                                        MsAstNode *body);
MsAstNode *ms_parser_parse_block_statement(MsParser *parser);
MsAstNode *ms_parser_parse_declaration(MsParser *parser);
void ms_parser_synchronize(MsParser *parser);
int ms_parser_consume_terminator(MsParser *parser, const char *message);

#endif