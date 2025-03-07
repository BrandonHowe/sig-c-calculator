#include <stdio.h>
#include <stdlib.h>

typedef struct StringView
{
    int length;
    const char* buffer;
} StringView;

StringView read_file(const char* file_path)
{
    FILE* file = fopen(file_path, "r"); // Open the file path in read mode
    if (!file)
    {
        printf("File could not be opened!\n");
        return (StringView){ 0 };
    }

    fseek(file, 0, SEEK_END); // Navigate to the end of the file
    int len = ftell(file); // Ask windows where we are
    rewind(file);

    char* buffer = malloc(len + 1);

    fread(buffer, 1, len, file);

    StringView view = (StringView){
        .length = len,
        .buffer = buffer
    };

    fclose(file);
    buffer[len] = 0;

    return view;
}

typedef enum TokenType
{
    TokenTypeInteger,
    TokenTypePlus,
    TokenTypeMinus,
    TokenTypeTimes,
    TokenTypeDivide
} TokenType;

typedef struct Token
{
    TokenType type;
    StringView text;
} Token;

typedef struct TokenizerOutput
{
    int token_count;
    Token* tokens;
} TokenizerOutput;

TokenizerOutput tokenize(StringView code)
{
    TokenizerOutput output = (TokenizerOutput) {
        .token_count = 0,
        .tokens = malloc(sizeof(Token) * 1000)
    };

    while (code.length > 0) // We have characters left to handle
    {
        char c = code.buffer[0];
        if (c == '+')
        {
            // Add the token to our list
            output.tokens[output.token_count] = (Token){
                .type = TokenTypePlus,
                .text = (StringView){
                    .length = 1,
                    .buffer = code.buffer
                }
            };
            output.token_count += 1;

            // Increment the buffer by 1
            code.buffer += 1;
            code.length -= 1;
        }
        else if (c == '-')
        {
            // Add the token to our list
            output.tokens[output.token_count] = (Token){
                .type = TokenTypeMinus,
                .text = (StringView){
                    .length = 1,
                    .buffer = code.buffer
                }
            };
            output.token_count += 1;

            // Increment the buffer by 1
            code.buffer += 1;
            code.length -= 1;
        }
        else if (c == '*')
        {
            // Add the token to our list
            output.tokens[output.token_count] = (Token){
                .type = TokenTypeTimes,
                .text = (StringView){
                    .length = 1,
                    .buffer = code.buffer
                }
            };
            output.token_count += 1;

            // Increment the buffer by 1
            code.buffer += 1;
            code.length -= 1;
        }
        else if (c == '/')
        {
            // Add the token to our list
            output.tokens[output.token_count] = (Token){
                .type = TokenTypeDivide,
                .text = (StringView){
                    .length = 1,
                    .buffer = code.buffer
                }
            };
            output.token_count += 1;

            // Increment the buffer by 1
            code.buffer += 1;
            code.length -= 1;
        }
        else if (c >= '0' && c <= '9') // If it's a digit
        {
            int number_length = 0;
            while (number_length < code.length && code.buffer[number_length] >= '0' && code.buffer[number_length] <= '9')
            {
                number_length += 1;
            }

            output.tokens[output.token_count] = (Token){
                .type = TokenTypeInteger,
                .text = (StringView){
                    .length = number_length,
                    .buffer = code.buffer
                }
            };
            output.token_count += 1;

            code.buffer += number_length;
            code.length -= number_length;
        }
        else // Anything else
        {
            code.buffer += 1;
            code.length -= 1;
        }
    }

    return output;
}

int main(void)
{
    StringView file = read_file("/Users/brandonhowe/Documents/Projects/SIGC-live-calculator/program.sigc");
    tokenize(file);

    return 0;
}
