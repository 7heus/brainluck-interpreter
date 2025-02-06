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
EXPORT char *toBf(const char *text);

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

EXPORT char *toBf(const char *text)
{
    int memIndex = 0;
    short currentVal = 0;
    char *output = (char *)malloc(1 * sizeof(char));
    int output_index = 0;

    for (size_t i = 0; i < strlen(text); i++)
    {
        int targetVal = text[i];
        short diff = targetVal - currentVal;

        if (diff != 0)
        {
            if (abs(diff) > 10)
            {
                int loopValue = abs(diff) / 10;
                int remainder = abs(diff) % 10;

                output = (char *)realloc(output, (output_index + 5) * sizeof(char));
                output[output_index++] = '+';
                output[output_index++] = '[';
                output[output_index++] = '>';
                for (int j = 0; j < loopValue; j++)
                {
                    output = (char *)realloc(output, (output_index + 2) * sizeof(char));
                    output[output_index++] = '+';
                }
                output = (char *)realloc(output, (output_index + 3) * sizeof(char));
                output[output_index++] = '<';
                output[output_index++] = '-';
                output[output_index++] = ']';

                for (int j = 0; j < remainder; j++)
                {
                    output = (char *)realloc(output, (output_index + 2) * sizeof(char));
                    output[output_index++] = (diff > 0) ? '+' : '-';
                }
            }
            else
            {
                for (int j = 0; j < abs(diff); j++)
                {
                    output = (char *)realloc(output, (output_index + 2) * sizeof(char));
                    output[output_index++] = (diff > 0) ? '+' : '-';
                }
            }
        }

        output = (char *)realloc(output, (output_index + 2) * sizeof(char));
        output[output_index++] = '.';

        currentVal = targetVal;
    }

    output = (char *)realloc(output, (output_index + 1) * sizeof(char));
    output[output_index] = '\0';

    return output;
}

static const unsigned char base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

EXPORT char *b64_encode(const unsigned char *src, int *len)
{
    unsigned char *out, *pos;
    const unsigned char *end, *in;

    int olen = 4 * ((len + 2) / 3);

    if (olen < len)
        return NULL;
    char *outstring = (char *)malloc(sizeof(char) * olen);
    out = (unsigned char *)&outstring[0];

    end = src + len;
    in = src;
    pos = out;

    while (end - in >= 3)
    {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
    }

    if (end - in)
    {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1)
        {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        }
        else
        {
            *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }

    return outstring;
}