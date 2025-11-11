//
// Created by andy1 on 24.10.2025.
//

#include "Lexer.h"

#include <utility>
#include "../Utils.h"
#include <iostream>

namespace Core {
    Lexer::Lexer(std::string source)
        : source(std::move(source)), pos(0), line(1), column(1) {}

    char Lexer::peek() const {
        if (pos >= source.size()) {
            return '\0';
        }
        return source[pos];
    }

	char Lexer::peek(int offset) const
	{
        int targetPos = pos + offset;
        if (targetPos >= source.size()) {
            return '\0';
        }
        return source[targetPos];
	}

	char Lexer::get() {
        if (pos >= source.size()) {
            return '\0';
        }
        char c = source[pos++];
        if (c == '\n') {
            line++;
            column = 1;
        }
        else {
            column++;
        }
        return c;
    }

    void Lexer::skipWhitespace() {
        while (std::isspace(peek())) {
            get();
        }
    }

    bool Lexer::isOperator(char c) {
        std::string value(1, c);
        if (operators.find(value) != std::end(operators)) {
            return true;
        }
        return false;
    }

	bool Lexer::match(const std::string& value)
	{
        for (size_t i = 0; i < value.size(); i++) {
            char c = value[i];
            char t = peek(i);
            if (c != t) return false;
        }
        return true;
	}

	Token Lexer::identifier() {
        int startColumn = column;
        std::string value;
        while (std::isalnum(peek()) || peek() == '_') {
            value += get();
        }

        if (keywords.find(value) != keywords.end()) {
            return {TokenType::Keyword, value, line, startColumn};
        }
        return {TokenType::Identifier, value, line, startColumn};
    }

	Token Lexer::number() {
        int startColumn = column;
        std::string value;
        bool hasDot = false;
        bool hasExp = false;

        while (true) {
            char c = peek();
            if (std::isdigit(static_cast<unsigned char>(c))) {
                value += get();
                continue;
            }
            if (c == '.' && !hasDot) {
                hasDot = true;
                value += get();
                continue;
            }
            if ((c == 'e' || c == 'E') && !hasExp) {
                hasExp = true;
                value += get();
                char next = peek();
                if (next == '+' || next == '-') {
                    value += get();
                }
                continue;
            }

            if ((c == 'f' || c == 'F' || c == 'l' || c == 'L') && (hasDot || hasExp)) {
                value += get();
                break;
            }
            break;
        }

        if (!value.empty()) {
			char lastChar = value.back();
            if (lastChar == 'f' || lastChar == 'F') {
				return { TokenType::FloatLiteral, value, line, startColumn };
            }
            if (lastChar == 'l' || lastChar == 'L') {
                // Treat long literals as integers for now
				std::cout << "Warning: 'long' literals are treated as integers in this lexer.\n";
                return { TokenType::IntegerLiteral, value, line, startColumn };
            }
            if (hasDot || hasExp) {
                return { TokenType::DoubleLiteral, value, line, startColumn };
            }
			return { TokenType::IntegerLiteral, value, line, startColumn };
        }
    }

    Token Lexer::string() {
        int startColumn = column;
        std::string value;
        get(); // Skip opening "

        while (true) {
            char c = peek();
            if (c == '\0') break;
            if (c == '"') {
                get();
                break;
            }
            if (c == '\\') {
                get(); // remove the escape slash
                char next = get();
                if (next == 'n') value += '\n';
                else if (next == 't') value += '\t';
                else if (next == 'r') value += '\r';
                else if (next == '"') value += '"';
                else value += next;
            }
            else {
                value += get();
            }
        }
        return {TokenType::StringLiteral, value, line, startColumn};
    }




    Token Lexer::symbol() {
        int startColumn = column;
        char c = get();
        return {TokenType::Symbol, std::string(1, c), line, startColumn};
    }

    Token Lexer::op() {
        int startColumn = column;
        char c = get();
        std::string value(1, c);
        char next = peek();
        while (isOperator(next)) {
            get();
            value += next;
            next = peek();
        }
        return {TokenType::Operator, value, line, startColumn};
    }

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
        while (true) {
            skipWhitespace();
            char c = peek();
            if (c == '\0') {
                tokens.emplace_back(TokenType::EndOfFile, "", line, column);
                break;
            }

            if (c == '"') {
                tokens.push_back(string());
            }
            else if (std::isalpha(c) || c == '_') {
                tokens.push_back(identifier());
            }
            else if (std::isdigit(c)) {
                tokens.push_back(number());
            }
            else if (isOperator(c)) {
                tokens.push_back(op());
            }
            else {
                tokens.push_back(symbol());
            }
        }
        return tokens;
    }
} // Lexer