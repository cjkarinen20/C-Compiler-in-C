#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h> 
#include <ctype.h>
#include <sys/stat.h>

#define MAX_SIZE 100 //Max length for input buffer

//--- Token Types ---
typedef enum 
{
    TOKEN_INT,
    TOKEN_VOID,
    TOKEN_RETURN,
    TOKEN_IDENTIFIER,
    TOKEN_CONSTANT,
    TOKEN_OPEN_PAREN,  // (
    TOKEN_CLOSE_PAREN, // )
    TOKEN_OPEN_BRACE,  // {
    TOKEN_CLOSE_BRACE, // }
    TOKEN_SEMICOLON,   // ;
    TOKEN_EOF,         // End of file
    TOKEN_ERROR        // Error state
} TokenType;

//--- Token Structure ---
typedef struct 
{
    TokenType type;
    char *lexeme;
} Token;

/**
 * @brief Turns a token type into a debug string.
 * @param type The token type to convert.
 * @return A pointer to the debug string representation of the token type.
 */
const char *token_to_debug_string(TokenType type) 
{
    switch (type) 
    {
        case TOKEN_INT: return "KEYWORD_INT";
        case TOKEN_VOID: return "KEYWORD_VOID";
        case TOKEN_RETURN: return "KEYWORD_RETURN";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_CONSTANT: return "CONSTANT";
        case TOKEN_OPEN_PAREN: return "OPEN_PAREN";
        case TOKEN_CLOSE_PAREN: return "CLOSE_PAREN";
        case TOKEN_OPEN_BRACE: return "OPEN_BRACE";
        case TOKEN_CLOSE_BRACE: return "CLOSE_BRACE";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    
    }
}

// --- Regular Expressions ---
// Identifier [a-zA-Z_]\w*\b
// Constant [0-9]+\b
// int keyword int\b
// void keyword void\b
// return keyword return\b
// Open parenthesis \(
// Close parenthesis \)
// Open brace {
// Close brace }
// Semicolon ;

//--- Lexer Functions ---

/**
 * @brief Skips whitespace characters in the input string.
 * @param input The input string to process.
 * @return A pointer to the first non-whitespace character in the input string.
 */
char *skip_whitespace(char *input)
{
    while (*input != '\0' && isspace((unsigned char)*input)) 
        input++;
    return input;
}

/**
 * @brief Skips comments in the input string.
 * @param input The input string to process.
 * @return A pointer to the first non-comment character in the input string.
 */
char *skip_comments(char *input)
{
    while (*input != '\0') 
    {
        if (input[0] == '/' && input[1] == '/') 
        {
            // Single-line comment
            while (*input != '\0' && *input != '\n') 
                input++;
        } 
        else if (input[0] == '/' && input[1] == '*') 
        {
            // Multi-line comment
            input += 2; // Skip the /*
            while (*input != '\0' && !(input[0] == '*' && input[1] == '/')) 
                input++;
            if (*input != '\0') 
                input += 2; // Skip the */
        } 
        else 
        {
            break; // No more comments to skip
        }
    }
    return input;
}

/**
 * @brief Checks for keywords and returns the appropriate token type.
 * @param lexeme The string to check.
 * @return The token type corresponding to the keyword, or TOKEN_IDENTIFIER if not a keyword.
 */
TokenType check_keyword(const char *lexeme) 
{
    if (strcmp(lexeme, "int") == 0) return TOKEN_INT;
    if (strcmp(lexeme, "void") == 0) return TOKEN_VOID;
    if (strcmp(lexeme, "return") == 0) return TOKEN_RETURN;
    return TOKEN_IDENTIFIER;
}

/**
 * @brief Gets the next token from the input string.
 * @param current_pos A pointer to the current position in the input string.
 * @return The next token.
 */
Token get_next_token(char **current_pos)
{
    //Skip whitespace and comments
    *current_pos = skip_whitespace(*current_pos);
    char *input = *current_pos;

    //Check for end of file
    if (*input == '\0') 
    {
        Token token = {TOKEN_EOF, NULL};
        return token;
    }

    //--- Simple Single-Character Tokens ---
    Token simple_token = {TOKEN_ERROR, strdup("")};

    switch (*input)
    {
        case '(': simple_token.type = TOKEN_OPEN_PAREN; break;
        case ')': simple_token.type = TOKEN_CLOSE_PAREN; break;
        case '{': simple_token.type = TOKEN_OPEN_BRACE; break;
        case '}': simple_token.type = TOKEN_CLOSE_BRACE; break;
        case ';': simple_token.type = TOKEN_SEMICOLON; break;
        default: break;
    }

    if (simple_token.type != TOKEN_ERROR) 
    {
        simple_token.lexeme = strndup(input, 1);
        (*current_pos)++;
        return simple_token;
    }

    //--- Identifiers and Constants---

    char *p = input;
    int len = 0;

    if (isalpha((unsigned char)*p) || *p == '_') 
    {
        // Identifier or keyword
        while (isalnum((unsigned char)*p) || *p == '_') 
        {
            p++;
            len++;
        }

        if (!isalnum((unsigned char)*p) && *p != '_') 
        {
            char *lexeme = strndup(input, len);
            Token token = {check_keyword(lexeme), lexeme};
            *current_pos += len;
            return token;
        } 
    } 
    
    p = input;
    len = 0;

    if (isdigit((unsigned char)*p)) 
    {
        p++;
        len++;
        
        // Constant
        while (isdigit((unsigned char)*p)) 
        {
            p++;
            len++;
        }

        if (!isalnum((unsigned char)*p) && *p != '_') 
        {
            char *lexeme = strndup(input, len);
            Token token = {TOKEN_CONSTANT, lexeme};
            *current_pos += len;
            return token;
        } 
    }

    while (*input != '\0' && !isspace((unsigned char)*input) && 
           *input != '(' && *input != ')' && *input != '{' && *input != '}' && *input != ';') 
    {
        input++;
        len++;
    }

    Token error_token = {TOKEN_ERROR, strndup(*current_pos, len > 0 ? len : 1)};

    *current_pos += len > 0 ? len : 1; 
    return error_token;
}


//--- Main Program ---

/**
 * @brief Reads an entire file into a dynamically allocated string.
 * @param filename The name of the file to read.
 * @return A pointer to the dynamically allocated string containing the file contents, or NULL on failure.
 */
char *read_file(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    if (!string) { fclose(f); return NULL; }

    fread(string, fsize, 1, f);
    fclose(f);
    string[fsize] = 0;
    
}

/**
 * @brief The main function for the compiler driver.
 * @param argc The argument count.
 * @param argv The argument vector.
 * @return EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char *argv[])
{
    char input_buffer[MAX_SIZE];

    while (fgets(input_buffer, MAX_SIZE, stdin) != NULL) 
    {
        char *token = strtok(input_buffer, " \t\n");
        int index = 0;
        while (token != NULL) 
        {
            
        }

    }if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    char *source_code = read_file(argv[1]);
    if (!source_code) {
        perror("Error reading file");
        return 1;
    }

    printf("--- Source Code ---\n%s\n-------------------\n", source_code);
    printf("--- Token List ---\n");

    char *current_pos = source_code;
    Token current_token;
    int error_count = 0;

    // Loop through the input until EOF
    do {
        current_token = get_next_token(&current_pos);
        
        // Print the token
        printf("%s", token_type_to_string(current_token.type));

        if (current_token.type == TOKEN_IDENTIFIER || current_token.type == TOKEN_CONSTANT || 
            current_token.type == TOKEN_KEYWORD_INT || current_token.type == TOKEN_KEYWORD_VOID || 
            current_token.type == TOKEN_KEYWORD_RETURN) 
        {
            printf(" (\"%s\")\n", current_token.lexeme);
        } else {
            printf("\n");
        }

        if (current_token.type == TOKEN_ERROR) {
            fprintf(stderr, "LEXER ERROR: Unrecognized token near '%s'\n", current_token.lexeme);
            error_count++;
        }
        
        // Free the dynamically allocated lexeme string
        free(current_token.lexeme);

    } while (current_token.type != TOKEN_EOF);

    printf("-------------------\n");
    if (error_count > 0) {
        printf("Lexing finished with %d errors.\n", error_count);
        free(source_code);
        return 1;
    }

    free(source_code);
    return 0;
}
