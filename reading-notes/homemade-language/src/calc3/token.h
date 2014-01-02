#ifndef __TOKEN_HEADER_H__
#define __TOKEN_HEADER_H__

enum TokenType {
  kTokenTypeInval = 0, 
  kTokenTypeNumber, 
  kTokenTypeAdd, 
  kTokenTypeSub, 
  kTokenTypeMul, 
  kTokenTypeDiv, 
  kTokenTypeCR, 
};

#define MAX_TOKEN_SIZE  (128)

typedef struct {
  int    type;
  double value;
  char   name[MAX_TOKEN_SIZE];
} token_t;

extern void set_line(const char* line);
extern void get_token(token_t* token);

#endif  /* __TOKEN_HEADER_H__ */
