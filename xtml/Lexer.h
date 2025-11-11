//////////////////////////////////////////////////////////////////////////
// C-Syntax lexer
// Created by andy16823 on 24.10.2025.
// Lexer from SageLang adapted for XTML
// SageLang License: MIT
// SageLang is a full programming language created by andy16823
// XTML is a markup language created by andy16823
//////////////////////////////////////////////////////////////////////////
#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

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
		: type(t), value(std::move(v)), line(l), column(c) {
	}
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
		"var",
		"return",
		"import",
		"for",
		"if",
		"else",
		"while",
		"continue",
		"break",
		"print"
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