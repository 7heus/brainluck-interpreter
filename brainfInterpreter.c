#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAPE_SIZE 30000

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

EXPORT char *interpreter(const char *code, const char *input);
EXPORT void free_memory(char *ptr);

EXPORT char *interpreter(const char *code, const char *input)
{
    unsigned char *tape = (unsigned char *)malloc(TAPE_SIZE * sizeof(char));
    if (tape == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    memset(tape, 0, TAPE_SIZE * sizeof(char));

    int pointer = 0;
    int inputPointer = 0;
    int stack[TAPE_SIZE];
    int currentStackSize = 0;
    int pc = 0;
    int outputted = 0;

    char *output = (char *)malloc(1 * sizeof(char));
    size_t codeLen = strlen(code);

    while (pc < codeLen)
    {
        char command = code[pc];
        switch (command)
        {
        case '>':
            if (pointer < TAPE_SIZE - 1)
            {
                pointer++;
            }
            else
            {
                fprintf(stderr, "Pointer moved out of bounds\n");
                free(tape);
                free(output);
                return NULL;
            }
            break;
        case '<':
            if (pointer > 0)
            {
                pointer--;
            }
            else
            {
                fprintf(stderr, "Pointer moved out of bounds\n");
                free(tape);
                free(output);
                return NULL;
            }
            break;
        case '+':
            tape[pointer] = (tape[pointer] + 1) % 256;
            break;
        case '-':
            tape[pointer] = (tape[pointer] - 1 + 256) % 256;
            break;
        case '.':
            char *newOutput = (char *)realloc(output, (outputted + 2) * sizeof(char));
            if (newOutput == NULL)
            {
                fprintf(stderr, "Memory allocation failed\n");
                free(tape);
                free(output);
                return NULL;
            }
            output = newOutput;
            output[outputted++] = (char)tape[pointer];
            output[outputted] = '\0';

            break;
        case ',':
            if (input[inputPointer] != '\0')
            {
                tape[pointer] = (unsigned char)input[inputPointer++];
            }
            else
            {
                tape[pointer] = 0;
            }
            break;
        case '[':
            if (tape[pointer] == 0)
            {
                int open = 1;
                while (open > 0)
                {
                    pc++;
                    if (code[pc] == '[')
                        open++;
                    if (code[pc] == ']')
                        open--;
                }
            }
            else
            {
                stack[currentStackSize++] = pc;
            }
            break;
        case ']':
            if (currentStackSize > 0)
            {
                if (tape[pointer] != 0)
                {
                    pc = stack[currentStackSize - 1];
                }
                else
                {
                    currentStackSize--;
                }
            }
            else
            {
                fprintf(stderr, "Unmatched ']' encountered\n");
                free(tape);
                free(output);
                return NULL;
            }
            break;
        }
        pc++;
    }
    output[outputted] = '\0';
    free(tape);
    return output;
}

EXPORT void free_memory(char *ptr)
{
    free(ptr);
}
