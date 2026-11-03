
# Lexical Analyzer in C

## Overview
A C-based lexical analyzer that reads a source file and identifies tokens, recording each token’s lexeme, type, and line number in a report.

## Features
- Recognizes keywords, identifiers, integer and floating-point constants
- Identifies operators, delimiters, string literals, and character literals
- Recognizes preprocessor directives and comments
- Reports the total number of tokens
- Writes the token report to `tokens.txt`
- Handles empty or whitespace-only input files

## Project Files
- `main.c` — prompts for the source filename and starts analysis
- `lexer.c` — scans the source file and generates the token report
- `lexer.h` — defines token types and shared declarations
- `input.c` — sample C source file for testing
- `tokens.txt` — sample token report

## Compile
```bash
gcc main.c lexer.c -o lexical_analyzer
```

## Run
```bash
./lexical_analyzer
```

When prompted, enter the C source filename, such as `input.c`. The program creates or overwrites `tokens.txt` with the analysis report.

## Concepts Used
C programming, file handling, character processing, strings, functions, enums, and token classification.
