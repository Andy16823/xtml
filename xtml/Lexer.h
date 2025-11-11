//
// Created by andy1 on 24.10.2025.
//

#ifndef SAGE_LEXER_H
#define SAGE_LEXER_H
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Core {
    enum class TokenType {
        Identifier,
        Keyword,
        IntegerLiteral,
        StringLiteral,
        FloatLiteral,
		DoubleLiteral,
        Symbol,
        Operator,
        EndOfFile,
    };

    struct Token {
        TokenType type;
        std::string value;
        int line;
        int column;

        Token(TokenType t, std::string  v, int l, int c)
            : type(t), value(std::move(v)), line(l), column(c) {}
    };

    class Lexer {
        public:
        Lexer(std::string  source);
        std::vector<Token> tokenize();

        private:
        std::string source;
        size_t pos;
        int line;
        int column;

        std::unordered_set<std::string> keywords = {
            "byte",
            "int",
			"float",
			"bool",
            "double",
            "void",
            "return",
            "import",
            "string",
            "for",
            "if",
            "else",
            "while",
            "continue",
            "break",
            "struct",
            "as"
        };

        std::unordered_set<std::string> operators = {
            "+",
            "-",
            "*",
            "/",
            "=",
            ">",
            "<",
            "&"
        };

        char peek() const;
        char peek(int offset) const;
        char get();
        void skipWhitespace();
        bool isOperator(char c);
        bool match(const std::string& value);

        Token identifier();
        Token number();
        Token string();
        Token symbol();
        Token op();

    };
} // Lexer

#endif //SAGE_LEXER_H