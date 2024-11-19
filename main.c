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
    DRIFT
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

// Function to check if character is a special character
int is_sc(char current) {
    return !isalnum(current) && !isspace(current);
}

void handle_slc(FILE *file, int *l, int *c) {
    char current_char;
    while ((current_char = fgetc(file)) != EOF && current_char != '\n') {
        (*c)++;
    }
    
    if (current_char == '\n') {
        (*l)++;
        *c = 1;
    }
}

void handle_mlc(FILE *file, int *l, int *c) {
    char current_char;
    while ((current_char = fgetc(file)) != EOF) {
        (*c)++;
        if (current_char == '*' && (current_char = fgetc(file)) == '/') {
            (*c)++;
            return;
        }
        if (current_char == '\n') {
            (*l)++;
            *c = 1;
        }
    }
}

// Function to check if character is a NUM and assigns NUM as the value of the token if yes
// Token *generate_number(char current, FILE *file) { // get number together (not read individually)
//     Token *token = malloc(sizeof(Token));
//     token->type = NUM;
//     token->token_type = "NUM";
//     char *value = malloc(sizeof(char) * 8);
//     int value_index = 0;

//     while (isdigit(current) && current != EOF) {
//         value[value_index] = current;
//         value_index++;
//         current = fgetc(file);
//     }

//     value[value_index] = '\0'; // Null terminate the string
//     token->value = value;

//     // Push the last non-digit character back onto the stream <- so that parentheses will not be eaten
//     if (current != EOF) {
//         ungetc(current, file);
//     }
    
//     return(token);
// }

// Function to check if the character is a bracket
int check_bracket(const char *token) {
    const char *brackets[] = {
        "[", "]", "{", "}", "(", ")"
    };

    for (int i = 0; i < (sizeof(brackets) / sizeof(brackets[0])); i++) {
        if (strcmp(token, brackets[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

// Function to check if character is a delimiter
int check_delimiter(const char *token) {
    const char *delimiters[] = {
        ";", ",", "\""
    };

    for (int i = 0; i < (sizeof(delimiters) / sizeof(delimiters[0])); i++) {
        if (strcmp(token, delimiters[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

int check_operator(const char *token) {
    const char *operators[] = {
        "+", "-", "*", "/", "%"
    };

    for (int i = 0; i < (sizeof(operators) / sizeof(operators[0])); i++) {
        if (strcmp(token, operators[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

// Checks if character is a bracket/delimiter and assigns it as value of the token if either
Token *generate_single_token(char current) {
    Token *token = malloc(sizeof(Token));
    char lexeme[2];
    lexeme[0] = current;
    lexeme[1] = '\0';

    if (check_bracket(lexeme)) {
        token->type = BRACKET;
        token->value = strdup(lexeme);
        token->token_type = "BRACKET";
    } else if (check_delimiter(lexeme)) {
        token->type = DELIMITER;
        token->value = strdup(lexeme);
        token->token_type = "DELIMITER";
    } else if (check_operator(lexeme)) {
        token->type = OPERATOR;
        token->value = strdup(lexeme);
        token->token_type = "OPERATOR";
    } else {
        free(token);
        return NULL;
    }

    return token;
} 

// Checks if token is a keyword
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

// Checks if token is a reserve word
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

// Checks if token is a identifier
int check_identifier(const char *token) {
    int i = 0;
    char current = token[i];
    int state = 1;

    // FSM for identifiers
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
}

// Checks which kind of token the token is (for identifier, keyword, or reserve word)
Token *generate_token(char current, FILE *file) {
    Token *token = malloc(sizeof(Token));
    char *lexeme = malloc(sizeof(char) * 32);
    int lexeme_index = 0;
    int has_digit = 0;
    int has_alpha = 0;
    int has_sc = 0;
    int has_decimal_point = 0;

    while (!(isspace(current)) && current != EOF) {
        lexeme[lexeme_index] = current;
        lexeme_index++;

        if (isdigit(current)) {
            has_digit = 1;
        }

        if (isalpha(current)) {
            has_alpha = 1;
        }

        if (is_sc(current)) {
            has_sc = 1;
        }

        if (current == '.') {
            // if (has_decimal_point) {
            //     // Para di sumobra decimal point
            //     // lexeme[lexeme_index - 1];
            //     printf("%s", lexeme);
            //     break;
            // }
            has_decimal_point += 1;
        } 
        // else {
        //     has_digit = 1;
        // }

        // if (isspace(current)) {
        //     break;
        // }

        current = fgetc(file);
    }

    lexeme[lexeme_index] = '\0'; // Null terminate the string

    // If walang following alphabet or special character 'yung digit, it's simply a NUM.
    if (has_digit && has_decimal_point == 1 && !has_alpha) {
        token->type = CONSTANT;
        token->value = strdup(lexeme);
        token->token_type = "CONSTANT (DRIFT)";
    } else if (has_digit && !has_alpha && !has_sc) {
        token->type = CONSTANT;
        token->value = strdup(lexeme);
        token->token_type = "CONSTANT (NUM)";
    } else if (check_keyword(lexeme)) {
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

    // Para hindi makain 'yung parentheses
    if (current != EOF) {
        ungetc(current, file);
    }

    return token;
}


// Token *generate_operator(char current) {
//     Token *token = malloc(sizeof(Token));
//     char lexeme[2];
//     lexeme[0] = current;
//     lexeme[1] = '\0';

//     if (check_operator(lexeme)) {
//         token->type = OPERATOR;
//         token->value = strdup(lexeme);
//         token->token_type = "OPERATOR";
//     } else {
//         free(token);
//         return NULL;
//     }

//     return token;
// } 

void generate_symbol_table(const Token *tokens, int token_count) {
    FILE *symbol_table = fopen("symbol_table.txt", "w");
    if (!symbol_table) {
        printf("Error: Could not create symbol table.");
        return;
    }

    for (int i = 0; i < token_count; i++) {
        fprintf(symbol_table, "Type: %d, Category: %s, Value: %s, Line: %d, Column: %d\n",
        tokens[i].type, tokens[i].token_type, tokens[i].value, tokens[i].l, tokens[i].c);
    }

    fclose(symbol_table);
}

// Lexer function
void lexer(FILE *file) {
    char current_char;
    int line = 1, column = 1;
    Token tokens[1000];
    int token_count = 0;

    // Checks each character in file
    while ((current_char = fgetc(file)) != EOF) {
        if (isspace(current_char)) {
            if (current_char == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            continue;
        }

        if (current_char == '/') {
            current_char = fgetc(file);
            // Single line comments start with //
            if (current_char == '/') {
                handle_slc(file, &line, &column);
                continue;
            // Multiline comments start with /*
            } else if (current_char == '*') {
                handle_mlc(file, &line, &column);
                continue;
            }
        }
        
        Token *token = NULL;

        token = generate_single_token(current_char);
        if (token != NULL) {
            token->l = line;
            token->c = column;
            tokens[token_count++] = *token;
            free(token);
            column++;
            continue;
        }
 
        // Checks if keyword/identifier/num/reserve word
        if (isalnum(current_char) || current_char == '_' || current_char == '#') {
            token = generate_token(current_char, file);
            token->l = line;
            token->c = column;
            tokens[token_count++] = *token;
            free(token);
            column += strlen(token->value);
            continue;
        }

        // if (is_sc(current_char)) {
        //     token = generate_operator(current_char);
        //     token->l = line;
        //     token->c = column;
        //     tokens[token_count++] = *token;
        //     free(token);
        //     column ++;
        //     continue;
        // }

        // Para sa mga hindi pumapasok sa if (so invalid)
        token = malloc(sizeof(Token));
        token->type = INVALID;
        token->token_type = "INVALID";
        token->value = malloc(2);
        token->value[0] = current_char;
        token->value[1] = '\0';
        token->l = line;
        token->c = column;
        tokens[token_count++] = *token;
        free(token);
        column++;
    }

    generate_symbol_table(tokens, token_count);
}

int check_filename(int arg_count, char *filename) {
    if (arg_count < 2) { // Checks if argument count is incomplete
        printf("Usage: <filename>\n");
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