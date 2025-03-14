#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct StringView
{
    char* buffer;
    int len;
} StringView;

StringView read_file(const char* file_name)
{
    FILE* file = fopen(file_name, "r"); // Open up our file for reading
    if (!file) // If the file isn't opened properly (file is nullptr), return
    {
        printf("Error opening file\n");
        return (StringView){ 0 };
    }

    fseek(file, 0, SEEK_END); // Move the "cursor" to the end of the file
    int size = ftell(file); // Ask for the current position of the "cursor"
    rewind(file); // Move the "cursor" back to the beginning

    // Allocate enough memory to store the file data, plus 1 byte for the null terminator
    // Remember to free the memory when we're done using it! It won't be in this function
    char* buffer = malloc(size + 1);
    if (!buffer) // Make sure memory allocation succeeds
    {
        printf("Malloc call failed");
        fclose(file);
        return (StringView){ 0 };
    }

    fread(buffer, 1, size, file); // From file, read in size characters, each character taking 1 byte, outputting it into buffer
    buffer[size] = '\0'; // Make sure the last character is a null terminator

    fclose(file); // Close our file handle when we're done with it
    return (StringView) {
        .buffer = buffer,
        .len = size
    };
}

typedef enum TokenType
{
    TokenTypeNull,
    TokenTypeInteger,
    TokenTypePlus,
    TokenTypeMinus,
    TokenTypeTimes,
    TokenTypeDivide,
    TokenTypeParenOpen,
    TokenTypeParenClose,
    TokenTypeIdentifier
} TokenType;

typedef struct Token
{
    TokenType type;
    StringView text;
} Token;

typedef struct TokenizerOutput
{
    Token* tokens;
    int token_count;

    int chars_consumed;

    StringView error;
} TokenizerOutput;

void add_token(StringView* code, TokenizerOutput* output, TokenType type, int token_length)
{
    // Generate the string for the token
    StringView token_str = (StringView){ .buffer = code->buffer, .len = token_length };
    // Generate the token itself
    Token token = (Token){ .type = type, .text = token_str };
    // Add the token to the output
    output->tokens[output->token_count] = token;
    output->token_count += 1;
    // Remove the token from our code
    code->buffer += token_length;
    code->len -= token_length;
}

// Converts code into a sequence of tokens
TokenizerOutput tokenize(StringView code)
{
    Token* tokens = malloc(1000 * sizeof(Token));
    TokenizerOutput output = (TokenizerOutput) {
        .tokens = tokens,
        .token_count = 0,
        .chars_consumed = 0,
        .error = (StringView){ 0 }
    };

    // Keep going until we hit the end of the file
    while (code.len > 0)
    {
        char c = code.buffer[0];
        switch (c)
        {
        case '+': add_token(&code, &output, TokenTypePlus, 1); break;
        case '-': add_token(&code, &output, TokenTypeMinus, 1); break;
        case '*': add_token(&code, &output, TokenTypeTimes, 1); break;
        case '/': add_token(&code, &output, TokenTypeDivide, 1); break;
        case '(': add_token(&code, &output, TokenTypeParenOpen, 1); break;
        case ')': add_token(&code, &output, TokenTypeParenClose, 1); break;
        default:
            {
                if (c >= '0' && c <= '9')
                {
                    // We want to find the length of the number
                    int number_len = 0;
                    // As long as the character is 0-9, add to the number length
                    while (number_len < code.len && code.buffer[number_len] >= '0' && code.buffer[number_len] <= '9')
                    {
                        number_len += 1;
                    }

                    // Now add the number token based on how long it is
                    add_token(&code, &output, TokenTypeInteger, number_len);
                    break;
                }
                // If all else fails, we advance forwards by 1 character
                code.buffer += 1;
                code.len -= 1;
                break;
            }
        }
    }

    return output;
}

typedef struct ASTNode
{
    TokenType type;
    int integer_value; // Only used when type is an integer
    struct ASTNode* left; // If type is an arithmetic operation, left addend/factor
    struct ASTNode* right; // If type is an arithmetic operation, right addend/factor
} ASTNode;

typedef struct TokenParser
{
    TokenizerOutput* tokenizer; // List of tokens that we got from lexing
    int current_token; // Which token we're currently looking at
} TokenParser;

// Looks at the next token, but doesn't get rid of it yet
Token* peek(TokenParser* parser) {
    // If we've reached the end, return NULL (a pointer that points to nothing)
    if (parser->current_token >= parser->tokenizer->token_count)
    {
         return NULL;
    }
    // Otherwise, we return the address of the next token
    return &parser->tokenizer->tokens[parser->current_token];
}

// Looks at the next token and gets rid of it
Token* consume(TokenParser* parser) {
    // If we've reached the end, return NULL (a pointer that points to nothing)
    if (parser->current_token >= parser->tokenizer->token_count)
    {
        return NULL;
    }
    Token* address = &parser->tokenizer->tokens[parser->current_token]; // Get the address of the next token
    parser->current_token += 1; // Move to the next token
    return address; // Return the address
}

ASTNode* parse_integer(TokenParser* parser);
ASTNode* parse_mul_div(TokenParser* parser);
ASTNode* parse_add_sub(TokenParser* parser);

// Requests memory for a new node, then stores the node data
ASTNode* create_node(TokenType type, int value, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->integer_value = value;
    node->left = left;
    node->right = right;
    return node;
}

ASTNode* parse_integer(TokenParser* parser) {
    Token* token = consume(parser);
    if (token->type == TokenTypeInteger) {
        // We need to convert the token from a string of characters ("123") to an integer (123)
        int total = 0;
        for (int i = 0; i < token->text.len; i++)
        {
            total *= 10; // Shifts the total left by 1 digit: 12 -> 120
            total += token->text.buffer[i] - '0'; // This converts a character into a digit ('3' -> 3)
            // In total this loop appends the next digit to the end: 12 -> 123
        }
        return create_node(TokenTypeInteger, total, NULL, NULL);
    } else if (token->type == TokenTypeParenOpen) {
        ASTNode* node = parse_add_sub(parser);
        consume(parser); // Expect closing parenthesis
        return node;
    }
    return NULL;
}

ASTNode* parse_mul_div(TokenParser* parser) {
    // Node starts off as an integer (the first factor in our multiplication)
    ASTNode* node = parse_integer(parser);
    Token* next_token = peek(parser); // Look at the next token, but don't consume it yet
    // If the next token is a times or a divide, we convert node into a mul/div operation.
    // Otherwise, we don't do anything and it stays an integer
    while (next_token && (next_token->type == TokenTypeTimes || next_token->type == TokenTypeDivide)) {
        next_token = consume(parser); // Now that we know the next token is a mul/div, we can consume it
        // Create a multiplication/division where the left is the first factor,
        // and the right is the next integer in the token list
        node = create_node(next_token->type, 0, node, parse_integer(parser));
        next_token = peek(parser); // Check for more * or / (e.g. 1 * 2 * 3)
    }
    return node;
}

ASTNode* parse_add_sub(TokenParser* parser) {
    // Multiplication comes first before addition, so if the first term is a multiplication operation we do that first
    // e.g. 1 * 2 + 3 -> the first thing is a multiplication, so we do that first
    ASTNode* node = parse_mul_div(parser);
    Token* next_token = peek(parser); // Look at the next token, but don't consume it yet
    // If the next token is an addition or subtraction, we convert node into a add/sub operation.
    // Otherwise, we don't do anything and it stays an integer
    while (next_token && (next_token->type == TokenTypePlus || next_token->type == TokenTypeMinus)) {
        next_token = consume(parser); // Now that we know the next token is an add/sub, we can consume it
        // We create an addition/subtraction where the left is the first addend, but we have to check
        // if the right is multiplication, so we can handle 1 + 2 * 3. For that expression, node would
        // be 1, and parse_mul_div(parser) would handle the 2 * 3
        node = create_node(next_token->type, 0, node, parse_mul_div(parser));
        next_token = peek(parser); // Check for more + or - (e.g. 1 + 2 + 3)
    }
    return node;
}

int evaluate_node(ASTNode node)
{
    switch (node.type)
    {
    case TokenTypeNull:
        assert(0 && "Null token type found");
        return 0;
    case TokenTypeInteger:
        return node.integer_value;
    case TokenTypePlus:
       return evaluate_node(*node.left) + evaluate_node(*node.right);
    case TokenTypeMinus:
       return evaluate_node(*node.left) - evaluate_node(*node.right);
    case TokenTypeTimes:
       return evaluate_node(*node.left) * evaluate_node(*node.right);
    case TokenTypeDivide:
       return evaluate_node(*node.left) / evaluate_node(*node.right);
    case TokenTypeIdentifier:
        break;
    default:
        break;
    }
    return 0;
}

int main(void)
{
    StringView content = read_file("program.sigc");
    TokenizerOutput output = tokenize(content);

    TokenParser parser = (TokenParser){ .tokenizer = &output, 0 };
    ASTNode* node = parse_add_sub(&parser);
    int result = evaluate_node(*node);

    printf("Program result: %i\n", result);

    // Free the file buffer now that we're done using it
    free(content.buffer);
    return 0;
}
