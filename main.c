
#include <stdio.h>
#include "lexer.h"

int main(void)
{
    char filename[100];

    printf("Enter the C source filename: ");
    scanf("%99s", filename);

    analyze_file(filename);

    return 0;
}
