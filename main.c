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

void lex(char *input, int length) {
}
