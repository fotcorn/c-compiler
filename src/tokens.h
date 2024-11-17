#pragma once

int TOKEN_LEFT_BRACE = 1;
int TOKEN_RIGHT_BRACE = 2;
int TOKEN_LEFT_PAREN = 3;
int TOKEN_RIGHT_PAREN = 4;
int TOKEN_LEFT_BRACKET = 5;
int TOKEN_RIGHT_BRACKET = 6;
int TOKEN_LITERAL_INT = 7;
int TOKEN_LITERAL_CHAR = 8;
int TOKEN_LITERAL_STRING = 9;
int TOKEN_IDENTIFIER = 10;
int TOKEN_RETURN = 11;
int TOKEN_IF = 12;
int TOKEN_ELSE = 13;
int TOKEN_WHILE = 14;
int TOKEN_STRUCT = 15;
int TOKEN_SEMICOLON = 16;
int TOKEN_COMMA = 17;
int TOKEN_PLUS = 18;
int TOKEN_MINUS = 19;
int TOKEN_MULTIPLY = 20;
int TOKEN_DIVIDE = 21;
int TOKEN_EQUAL = 22;
int TOKEN_EQUAL_EQUAL = 23;
int TOKEN_NOT_EQUAL = 24;
int TOKEN_LESS = 25;
int TOKEN_LESS_EQUAL = 26;
int TOKEN_GREATER = 27;
int TOKEN_GREATER_EQUAL = 28;
int TOKEN_PERIOD = 29;
int TOKEN_LOGICAL_OR = 30;
int TOKEN_LOGICAL_AND = 31;
int TOKEN_AMPERSAND = 32;

struct Token {
    int type;
    int start;
    int end;
    int line;
};

struct TokenArray {
    struct Token *tokens;
    int capacity;
    int count;
};
