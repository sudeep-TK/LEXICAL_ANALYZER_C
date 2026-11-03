
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

/* C keywords */
static const char *keywords[] =
{
    "auto", "break", "case", "char", "const", "continue",
    "default", "do", "double", "else", "enum", "extern",
    "float", "for", "goto", "if", "inline", "int", "long",
    "register", "restrict", "return", "short", "signed",
    "sizeof", "static", "struct", "switch", "typedef",
    "union", "unsigned", "void", "volatile", "while",
    "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex",
    "_Generic", "_Imaginary", "_Noreturn", "_Static_assert",
    "_Thread_local"
};

/* Check whether a word is a keyword */
static int is_keyword(const char *word)
{
    size_t i;
    size_t count = sizeof(keywords) / sizeof(keywords[0]);

    for (i = 0; i < count; i++)
    {
        if (strcmp(word, keywords[i]) == 0)
            return 1;
    }

    return 0;
}

/* Return readable token names */
const char *get_token_name(TokenType type)
{
    switch (type)
    {
        case KEYWORD:           return "Keyword";
        case IDENTIFIER:        return "Identifier";
        case INTEGER_CONSTANT:  return "Integer Constant";
        case FLOAT_CONSTANT:    return "Float Constant";
        case OPERATOR:          return "Operator";
        case DELIMITER:         return "Delimiter";
        case STRING_LITERAL:    return "String Literal";
        case CHARACTER_LITERAL: return "Character Literal";
        case PREPROCESSOR:      return "Preprocessor";
        case COMMENT:           return "Comment";
        default:                return "Unknown";
    }
}

/* Write one token as a formatted table row */
static void write_token(FILE *out, int *count,
                        const char *lexeme, TokenType type, int line)
{
    (*count)++;

    fprintf(out, "%-5d %-7d %-30.30s %-22s\n",
            *count, line, lexeme, get_token_name(type));
}

/* Add a character without overflowing the token buffer */
static void append_char(char *token, int *i, int ch)
{
    if (*i < MAX_TOKEN_SIZE - 1)
        token[(*i)++] = (char)ch;
}

/* Perform lexical analysis on a C source file */
void analyze_file(const char *filename)
{
    FILE *fp;
    FILE *out;
    int ch, next;
    int line = 1;
    int i;
    int has_content = 0;
    int token_count = 0;
    char token[MAX_TOKEN_SIZE];

    /* Open source file */
    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        printf("\nError: Cannot open input file '%s'.\n", filename);
        return;
    }

    /* Check for empty or whitespace-only file */
    while ((ch = fgetc(fp)) != EOF)
    {
        if (!isspace((unsigned char)ch))
        {
            has_content = 1;
            break;
        }
    }

    if (!has_content)
    {
        printf("\nError: Input file is empty or contains only whitespace.\n");
        fclose(fp);
        return;
    }

    rewind(fp);

    /* Create output report */
    out = fopen("tokens.txt", "w");

    if (out == NULL)
    {
        printf("\nError: Cannot create tokens.txt.\n");
        fclose(fp);
        return;
    }

    /* Report heading */
    fprintf(out, "====================================================================\n");
    fprintf(out, "                  LEXICAL ANALYZER - TOKEN REPORT\n");
    fprintf(out, "====================================================================\n");
    fprintf(out, "Source File : %s\n", filename);
    fprintf(out, "--------------------------------------------------------------------\n");
    fprintf(out, "%-5s %-7s %-30s %-22s\n",
            "No.", "Line", "Lexeme", "Token Type");
    fprintf(out, "--------------------------------------------------------------------\n");

    /* Read source file character by character */
    while ((ch = fgetc(fp)) != EOF)
    {
        if (ch == '\n')
        {
            line++;
            continue;
        }

        if (isspace((unsigned char)ch))
            continue;

        /* Preprocessor directive */
        if (ch == '#')
        {
            int start_line = line;

            i = 0;
            append_char(token, &i, ch);

            while ((ch = fgetc(fp)) != EOF && ch != '\n')
                append_char(token, &i, ch);

            token[i] = '\0';

            write_token(out, &token_count, token,
                        PREPROCESSOR, start_line);

            if (ch == '\n')
                line++;

            continue;
        }

        /* Identifier or keyword */
        if (isalpha((unsigned char)ch) || ch == '_')
        {
            i = 0;
            append_char(token, &i, ch);

            while ((ch = fgetc(fp)) != EOF &&
                   (isalnum((unsigned char)ch) || ch == '_'))
            {
                append_char(token, &i, ch);
            }

            token[i] = '\0';

            if (ch != EOF)
                ungetc(ch, fp);

            write_token(out, &token_count, token,
                        is_keyword(token) ? KEYWORD : IDENTIFIER, line);
            continue;
        }

        /* Integer or floating-point constant */
        if (isdigit((unsigned char)ch))
        {
            int is_float = 0;
            int dot_seen = 0;

            i = 0;
            append_char(token, &i, ch);

            while ((ch = fgetc(fp)) != EOF &&
                   (isdigit((unsigned char)ch) || ch == '.'))
            {
                if (ch == '.')
                {
                    if (dot_seen)
                    {
                        /* Preserve the extra dot for the next token */
                        ungetc(ch, fp);
                        break;
                    }

                    dot_seen = 1;
                    is_float = 1;
                }

                append_char(token, &i, ch);
            }

            token[i] = '\0';

            if (ch != EOF && !(ch == '.' && dot_seen &&
                               token[i - 1] == '.'))
            {
                ungetc(ch, fp);
            }

            write_token(out, &token_count, token,
                        is_float ? FLOAT_CONSTANT : INTEGER_CONSTANT, line);
            continue;
        }

        /* Floating-point constant beginning with a dot, e.g. .5 */
        if (ch == '.')
        {
            next = fgetc(fp);

            if (next != EOF && isdigit((unsigned char)next))
            {
                i = 0;
                append_char(token, &i, '.');
                append_char(token, &i, next);

                while ((ch = fgetc(fp)) != EOF &&
                       isdigit((unsigned char)ch))
                {
                    append_char(token, &i, ch);
                }

                token[i] = '\0';

                if (ch != EOF)
                    ungetc(ch, fp);

                write_token(out, &token_count, token, FLOAT_CONSTANT, line);
            }
            else
            {
                if (next != EOF)
                    ungetc(next, fp);

                write_token(out, &token_count, ".", DELIMITER, line);
            }

            continue;
        }

        /* String literal */
        if (ch == '"')
        {
            int escaped = 0;
            int closed = 0;
            int start_line = line;

            i = 0;
            append_char(token, &i, ch);

            while ((ch = fgetc(fp)) != EOF)
            {
                append_char(token, &i, ch);

                if (ch == '\n')
                    line++;

                if (escaped)
                {
                    escaped = 0;
                    continue;
                }

                if (ch == '\\')
                {
                    escaped = 1;
                    continue;
                }

                if (ch == '"')
                {
                    closed = 1;
                    break;
                }
            }

            token[i] = '\0';

            write_token(out, &token_count, token,
                        closed ? STRING_LITERAL : UNKNOWN, start_line);
            continue;
        }

        /* Character literal */
        if (ch == '\'')
        {
            int escaped = 0;
            int closed = 0;
            int start_line = line;

            i = 0;
            append_char(token, &i, ch);

            while ((ch = fgetc(fp)) != EOF)
            {
                append_char(token, &i, ch);

                if (ch == '\n')
                    line++;

                if (escaped)
                {
                    escaped = 0;
                    continue;
                }

                if (ch == '\\')
                {
                    escaped = 1;
                    continue;
                }

                if (ch == '\'')
                {
                    closed = 1;
                    break;
                }
            }

            token[i] = '\0';

            write_token(out, &token_count, token,
                        closed ? CHARACTER_LITERAL : UNKNOWN, start_line);
            continue;
        }

        /* Comments or division operator */
        if (ch == '/')
        {
            next = fgetc(fp);

            /* Single-line comment */
            if (next == '/')
            {
                int start_line = line;

                i = 0;
                append_char(token, &i, '/');
                append_char(token, &i, '/');

                while ((ch = fgetc(fp)) != EOF && ch != '\n')
                    append_char(token, &i, ch);

                token[i] = '\0';

                write_token(out, &token_count, token, COMMENT, start_line);

                if (ch == '\n')
                    line++;

                continue;
            }

            /* Multi-line comment */
            if (next == '*')
            {
                int previous = 0;
                int closed = 0;
                int start_line = line;

                i = 0;
                append_char(token, &i, '/');
                append_char(token, &i, '*');

                while ((ch = fgetc(fp)) != EOF)
                {
                    if (ch == '\n')
                        line++;

                    append_char(token, &i, ch);

                    if (previous == '*' && ch == '/')
                    {
                        closed = 1;
                        break;
                    }

                    previous = ch;
                }

                token[i] = '\0';

                write_token(out, &token_count, token,
                            closed ? COMMENT : UNKNOWN, start_line);
                continue;
            }

            if (next != EOF)
                ungetc(next, fp);

            /* Division or division-assignment */
            next = fgetc(fp);

            if (next == '=')
            {
                write_token(out, &token_count, "/=", OPERATOR, line);
            }
            else
            {
                if (next != EOF)
                    ungetc(next, fp);

                write_token(out, &token_count, "/", OPERATOR, line);
            }

            continue;
        }

        /* Operators */
        if (strchr("+-*=<>!%&|^~", ch) != NULL)
        {
            int start_line = line;

            token[0] = (char)ch;
            token[1] = '\0';

            next = fgetc(fp);

            if (next != EOF &&
                ((next == '=' && strchr("+-*=<>!%&|^", ch) != NULL) ||
                 (ch == '+' && next == '+') ||
                 (ch == '-' && (next == '-' || next == '>')) ||
                 (ch == '&' && next == '&') ||
                 (ch == '|' && next == '|') ||
                 (ch == '<' && next == '<') ||
                 (ch == '>' && next == '>')))
            {
                token[1] = (char)next;
                token[2] = '\0';

                if (ch == '<' && next == '<')
                {
                    int third = fgetc(fp);

                    if (third == '=')
                    {
                        token[2] = '=';
                        token[3] = '\0';
                    }
                    else if (third != EOF)
                    {
                        ungetc(third, fp);
                    }
                }
                else if (ch == '>' && next == '>')
                {
                    int third = fgetc(fp);

                    if (third == '=')
                    {
                        token[2] = '=';
                        token[3] = '\0';
                    }
                    else if (third != EOF)
                    {
                        ungetc(third, fp);
                    }
                }
            }
            else if (next != EOF)
            {
                ungetc(next, fp);
            }

            write_token(out, &token_count, token, OPERATOR, start_line);
            continue;
        }

        /* Delimiters */
        if (strchr("(){}[];,.:?", ch) != NULL)
        {
            token[0] = (char)ch;
            token[1] = '\0';

            write_token(out, &token_count, token, DELIMITER, line);
            continue;
        }

        /* Unknown character */
        token[0] = (char)ch;
        token[1] = '\0';

        write_token(out, &token_count, token, UNKNOWN, line);
    }

    /* Report footer */
    fprintf(out, "--------------------------------------------------------------------\n");
    fprintf(out, "Total Tokens: %d\n", token_count);
    fprintf(out, "====================================================================\n");

    fclose(fp);
    fclose(out);

    printf("\n");
    printf("====================================================\n");
    printf("       LEXICAL ANALYSIS COMPLETED SUCCESSFULLY\n");
    printf("====================================================\n");
    printf("Source File  : %s\n", filename);
    printf("Total Tokens : %d\n", token_count);
    printf("Output File  : tokens.txt\n");
    printf("====================================================\n");
}
