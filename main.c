#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> // to use isalpha() and isdigit()
#include <string.h> // to use strcmp() and strchr()

typedef enum {
    NUM,
    IDENTIFIER,
    KEYWORD,
    RESERVE_WORD,
    CONSTANT,
    NOISE_WORD,
    COMMENT,
    OPERATOR,
    DELIMITER,
    BRACKET,
    INVALID,
} TokenType;

typedef struct {
    TokenType type;
    char *token_type;
    char *value;
    int l, c;
} Token;

// Functions
// Function to check if the filetype corresponds to Sheesh#
int check_extension(const char* filename, const char *file_ext) {
    const char *name_end = strrchr(filename, '.'); // Returns a pointer to the last occurrence of '.' through strchr()
    const char *ext = name_end + 1; // Uses pointer arithmetic to get the substring after '.'

    /** 
        If the pointer to the '.' does not exist, name_end will return NULL, which will return false.
        strcmp() ccompares the extension of the given file to the required extension (filetype)
    **/

   // Checks if the file has an extension
    if (name_end == NULL || *ext == '\0') {
        printf("Usage: <filename>.<extension>\n");
        exit(1);
    }

    return name_end && strcmp(ext, file_ext) == 0;
}

Token *generate_number(char current, FILE *file) { // get number together (not read individually)
    Token *token = malloc(sizeof(Token));
    token->type = NUM;
    token->token_type = "NUM";
    char *value = malloc(sizeof(char) * 8);
    int value_index = 0;

    while (isdigit(current) && current != EOF) {
        value[value_index] = current;
        value_index++;
        current = fgetc(file);
    }

    value[value_index] = '\0'; // Null terminate the string
    token->value = value;

    // Push the last non-digit character back onto the stream <- so that parentheses will not be eaten
    if (current != EOF) {
        ungetc(current, file);
    }
    
    return(token);
}

int check_keyword(const char *token) {
    const char *keywords[] = {
        "bounce", "car", "do", "drift", "empty", "ex", "flip", "frozen", "group", "if", "input", "jump",
        "legit", "locked", "lockin", "long", "meanwhile", "nickname", "num", "open", "other", "out", "outside", "pl",
        "rep", "scenario", "short", "standard", "stop", "team", "text", "vibe"
    };

    for (int i = 0; i < (sizeof(keywords) / sizeof(keywords[0])); i++) {
        if (strcmp(token, keywords[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

int check_rw(const char *token) {
    const char *rws[] = {
        "always", "cap", "cont", "nocap", "toptier"
    };

    for (int i = 0; i < (sizeof(rws) / sizeof(rws[0])); i++) {
        if (strcmp(token, rws[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

int check_identifier(const char *token) {
    int i = 0;
    char current = token[i];
    int state = 1;

    while (current != '\0') {
        switch(state) {
            case 1: // Q1
                if (isalpha(current) || current == '_' || current == '#') {
                    state = 3;
                } else {
                    return 0;
                }
                break;
            case 3: // Q3
                if (isalnum(current)) {
                    state = 3;
                } else if (current == '#' || current == '_') {
                    state = 4;
                } else {
                    return 0;
                }
                break;
            case 4: // Q4
                if (isalnum(current)) {
                    state = 3;
                } else if (current == '#' || current == '_') {
                    state = 4;
                } else {
                    return 0;
                }
                break;
            default:
                return 0;
        }

        current = token[++i];
    }

    return state == 3;
    // // Identifiers must start with an English alphabet capital or lowercase letters, underscore, or sharp, but not with digits
    // if (!(isalpha(token[0]) || token[0] == '_' || token[0] == '#')) {
    //     printf("Identifier does not start with alphabet or _ or #!");
    //     return 0;
    // }

    // // Identifiers can include English alphabet capital and lowercase letters, digits.
    // // Identifiers cannot include special characters except only underscore (_) and sharp (#) characters.
    // for (int i = 1; token[i] != '\0'; i++) {
    //     if (!(isalnum(token[i])) || token[i] == '_' || token[i] == '#') {
    //         printf("Includes special characters other than underscore/sharp!");
    //         return 0;
    //     }
    // }

    // // If an identifier has an underscore (_) or sharp (#) characters, it must follow a single or multiple English alphabet capital or lowercase letters, or digits to be considered a valid identifier.
    // for (int i = 0; token[i] != '\0'; i++) {
    //     if ((token[i] == '_' || token[i] == '#') && !isalnum(token[i+1])) {
    //         printf("Special character is not followed by alnum!");
    //         return 0;
    //     }
    // }

    // // Spaces are not allowed in an identifier.
    // if (strchr(token, ' ') != NULL) {
    //     printf("There's a whitespace!");
    //     return 0;
    // }


    // // Keywords and reserved keywords of the language cannot be used as identifiers.
    // if (check_keyword(token) || check_rw(token)) {
    //     printf("It's a reserve word/token!");
    //     return 0;
    // }

    // return 1;
}

Token *generate_token(char current, FILE *file) {
    Token *token = malloc(sizeof(Token));
    char *lexeme = malloc(sizeof(char) * 32);
    int lexeme_index = 0;

    while ((isalnum(current) || current == '_' || current == '#') && current != EOF) {
        lexeme[lexeme_index] = current;
        lexeme_index++;
        current = fgetc(file);
    }

    lexeme[lexeme_index] = '\0'; // Null terminate the string

    if (check_keyword(lexeme)) {
        token->type = KEYWORD;
        token->value = strdup(lexeme);
        token->token_type = "KEYWORD";
    } else if (check_rw(lexeme)) {
        token->type = RESERVE_WORD;
        token->value = strdup(lexeme);
        token->token_type = "RESERVE WORD";
    } else if (check_identifier(lexeme)) {
        token->type = IDENTIFIER;
        token->value = strdup(lexeme);
        token->token_type = "IDENTIFIER";
    } else {
        token->type = INVALID;
        token->value = strdup(lexeme);
        token->token_type = "INVALID";
    }

    // Push the last non-digit character back onto the stream
    if (current != EOF) {
        ungetc(current, file);
    }

    return token;
}

void generate_symbol_table(const Token *tokens, int token_count) {
    FILE *out = fopen("symbol_table.txt", "w");
    if (!out) {
        fprintf(stderr, "Error: Could not create symbol table.\n");
        return;
    }

    for (int i = 0; i < token_count; i++) {
        fprintf(out, "Type: %d, Category: %s, Value: %s, Line: %d, Column: %d\n",
            tokens[i].type, tokens[i].token_type, tokens[i].value, tokens[i].l, tokens[i].c);
    }

    fclose(out);
}

void lexer(FILE *file) {
    char current_char;
    int line = 1, column = 1;
    Token tokens[1000];
    int token_count = 0;

    while ((current_char = fgetc(file)) != EOF) { // Checks each character (note: put this in separate function)
        if (isspace(current_char)) {
            if (current_char == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            continue;
        }

        Token token;
        token.l = line;
        token.c = column;

        if (current_char == ';') {
            token.type = DELIMITER;
            token.token_type = "DELIMITER";
            token.value = strdup(&current_char);
        } else if (current_char == '(') {
            token.type = BRACKET;
            token.token_type = "BRACKET";
            token.value = strdup(&current_char);
        } else if (current_char == ')') {
            token.type = BRACKET;
            token.token_type = "BRACKET";
            token.value = strdup(&current_char);
        } else if (isalpha(current_char) || current_char == '_' || current_char == '#') {
            Token *test_token = generate_token(current_char, file);
            token.type = test_token->type;
            token.token_type = strdup(test_token->token_type);
            token.value = strdup(test_token->value);
            // Token *test_token = generate_token(current_char, file);
            // printf("TEST %s VALUE: %s\n", test_token->token_type, test_token->value);
            free(test_token->value);
            free(test_token);
        } else if (isdigit(current_char)) {
            Token *test_token = generate_number(current_char, file);
            token.type = test_token->type;
            token.token_type = strdup(test_token->token_type);
            token.value = strdup(test_token->value);
            // printf("TEST TOKEN VALUE: %s\n", test_token->value);
            free(test_token->value);
            free(test_token);
        } 
        // printf("%c", current_char);
        // current_char = fgetc(file);
        
        tokens[token_count++] = token;
        column++;
    }

    generate_symbol_table(tokens, token_count);
}

int check_filename(int arg_count, char *filename) {
    if (arg_count < 2) { // Checks if argument count is incomplete
        printf("Usage: %s <filename>\n");
        exit(1);
    }

    if (!check_extension(filename, "shs")) { // Checks if .shs filetype
        printf("Error: Filetype must be '.shs'. Please try again.");
        exit(1);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    FILE *file;

    // Checks if argument (filename) is in proper format and is correct filetype
    check_filename(argc, argv[1]); 

    file = fopen(argv[1], "r"); // Open file
    if (!file) { // If file does not exist
        printf("Error: File '%s' does not exist.", argv[1]);
        return 1;
    }

    lexer(file);
    fclose(file);
    return 0;
}

