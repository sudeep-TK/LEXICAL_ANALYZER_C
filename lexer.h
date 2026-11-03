
#ifndef LEXER_H
#define LEXER_H

#define MAX_TOKEN_SIZE 100

typedef enum
{
    KEYWORD,
    IDENTIFIER,
    INTEGER_CONSTANT,
    FLOAT_CONSTANT,
    OPERATOR,
    DELIMITER,
    STRING_LITERAL,
    CHARACTER_LITERAL,
    PREPROCESSOR,
    COMMENT,
    UNKNOWN
} TokenType;

typedef struct
{
    char lexeme[MAX_TOKEN_SIZE];
    TokenType type;
    int line_number;
} Token;

void analyze_file(const char *filename);
const char *get_token_name(TokenType type);

#endif
