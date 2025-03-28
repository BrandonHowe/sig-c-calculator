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
    if (file == NULL)
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

typedef struct ASTNode
{
    TokenType type;
    int integer_value;
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

typedef struct Parser
{
    TokenizerOutput* tokenizer_output; // Output generated by the lexer
    int current_token_index; // What token we're currently looking at
} Parser;

// Looks at the current token and returns a pointer to it
Token* peek(Parser* parser)
{
    if (parser->current_token_index >= parser->tokenizer_output->token_count)
    {
        return NULL;
    }

    return &parser->tokenizer_output->tokens[parser->current_token_index];
}

// Look at the current token, and then move to the next one
Token* consume(Parser* parser)
{
    if (parser->current_token_index >= parser->tokenizer_output->token_count)
    {
        return NULL;
    }

    Token* token_address = &parser->tokenizer_output->tokens[parser->current_token_index]; // Look at the current token
    parser->current_token_index += 1; // Move to the next token
    return token_address;
}

ASTNode* create_node(TokenType type, int value, ASTNode* left, ASTNode* right)
{
    ASTNode* node = malloc(sizeof(ASTNode)); // Ask the computer for somewhere to store the node
    if (node == NULL) return NULL; // Not really necessary but sometimes nice to have
    node->type = type;
    node->integer_value = value;
    node->left = left;
    node->right = right;
    return node;
}

ASTNode* parse_integer(Parser* parser)
{
    Token* token = consume(parser);
    if (token->type == TokenTypeInteger)
    {
        // We need to turn a string into an integer value!
        int total = 0;
        for (int i = 0; i < token->text.length; i++) // Loop over our string
        {
            total *= 10; // Shift the total to the left by 1
            char current_digit_char = token->text.buffer[i];
            total += current_digit_char - '0';
        }
        return create_node(token->type, total, NULL, NULL);
    }
    return NULL;
}

ASTNode* parse_mul_div(Parser* parser)
{

}

ASTNode* parse_add_sub(Parser* parser)
{

}

int main(void)
{
    StringView file = read_file("/Users/brandonhowe/Documents/Projects/SIGC-live-calculator/program.sigc");
    TokenizerOutput tokenizer_output = tokenize(file);

    Parser parser = (Parser){
        .tokenizer_output = &tokenizer_output,
        .current_token_index = 0 // We start at the beginning
    };

    ASTNode* int_node = parse_integer(&parser);
    ASTNode* broken_node = parse_integer(&parser);
    ASTNode* int_node_2 = parse_integer(&parser);

    return 0;
}