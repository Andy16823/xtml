#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Lexer.h"
#include "ASTNode.h"

class Parser
{
private:
	std::vector<Token> m_tokens;
	size_t m_pos;

	Token& peek();
	Token& peek(int offset);
	Token& get();
	bool match(TokenType type, std::string value);

public:
	Parser(const std::vector<Token>& tokens)  	: m_tokens(tokens), m_pos(0) {
	}

	std::unique_ptr<ASTNode> parse();
	std::unique_ptr<FunctionNode> parseFunction();		// Parse function definitions
	std::unique_ptr<BlockNode> parseBlock();			// Parse code blocks
	std::unique_ptr<StmtNode> parseStatement();			// Parse individual statements
	std::unique_ptr<ExprNode> parseExpression();		// Parse expressions
	std::unique_ptr<ExprNode> parsePrimary();			// Parse primary expressions


	std::unique_ptr<VarDeclNode> parseVarDeclaration(const Token& token); // Parse variable declarations
	std::unique_ptr<ExprStatementNode> parseExprStatement(const Token& token); // Parse expression statements

};

