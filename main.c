int TOKEN_LEFT_BRACE = 1;
int TOKEN_RIGHT_BRACE = 2;
int TOKEN_LEFT_PAREN = 3;
int TOKEN_RIGHT_PAREN = 4;
int TOKEN_LITERAL_INT = 5;
int TOKEN_LITERAL_CHAR = 6;
int TOKEN_LITERAL_STRING = 7;
int TOKEN_IDENTIFIER = 8;
int TOKEN_RETURN = 9;
int TOKEN_IF = 10;
int TOKEN_ELSE = 11;
int TOKEN_WHILE = 12;
int TOKEN_STRUCT = 13;
int TOKEN_SEMICOLON = 14;
int TOKEN_COMMA = 15;

struct Token {
    int type;
    int start;
    int end;
};

struct TokenArray {
    struct Token *tokens;
    int capacity;
    int count;
};

struct TokenArray create_token_array() {
    struct TokenArray arr;
    arr.capacity = 8;
    arr.count = 0;
    arr.tokens = malloc(arr.capacity * sizeof(struct Token));
    return arr;
}

void add_token(struct TokenArray *arr, struct Token token) {
    if (arr->count >= arr->capacity) {
        arr->capacity *= 2;
        arr->tokens = realloc(arr->tokens, arr->capacity * sizeof(struct Token));
    }
    arr->tokens[arr->count++] = token;
}

struct TokenArray lex(char *input, int length) {
    struct TokenArray tokens = create_token_array();
    return tokens;
}
