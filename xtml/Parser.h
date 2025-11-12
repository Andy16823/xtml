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

	std::unique_ptr<XtmlBlockNode> parse();
	std::unique_ptr<FunctionNode> parseFunction();		// Parse function definitions
	std::unique_ptr<BlockNode> parseBlock();			// Parse code blocks
	std::unique_ptr<BlockNode> parseBracketBlock();
	std::unique_ptr<StmtNode> parseStatement();			// Parse individual statements
	std::unique_ptr<ExprNode> parseExpression();		// Parse expressions
	std::unique_ptr<ExprNode> parsePrimary();			// Parse primary expressions


	std::unique_ptr<VarDeclNode> parseVarDeclaration(const Token& token); // Parse variable declarations
	std::unique_ptr<ExprStatementNode> parseExprStatement(const Token& token); // Parse expression statements
	std::unique_ptr<IfStatementNode> parseIfStatement(const Token& token); // Parse if statements 
	std::unique_ptr<HtmlStmtRootNode> parseHtmlRootStatement(const Token& token); // Parse HTML root statements
	std::unique_ptr<HtmlStmtNode> parseHtmlStatement(const Token& token); // Parse HTML statements
	std::unique_ptr<HtmlTextNode> parseHtmlTextNode(const Token& token); // Parse HTML text nodes

	// Loops
	std::unique_ptr<ForNode> parseForStatement(const Token& token); // Parse for loops
};

