#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Lexer.h"
#include "ast/ASTNode.h"

class Parser
{
private:
	std::vector<Token> m_tokens;
	size_t m_pos;

	Token& peek();
	Token& peek(int offset);
	Token& get();
	bool match(TokenType type, std::string value);
	bool match(TokenType type, std::string value, size_t range);

public:
	Parser(const std::vector<Token>& tokens)  	: m_tokens(tokens), m_pos(0) {
	}

	std::unique_ptr<XtmlBlockNode> parse();
	std::unique_ptr<FunctionDeclNode> parseFunction();		// Parse function definitions
	std::unique_ptr<BlockNode> parseBlock();			// Parse code blocks
	std::unique_ptr<BlockNode> parseBracketBlock();
	std::unique_ptr<StmtNode> parseStatement();			// Parse individual statements
	std::unique_ptr<ExprNode> parseExpression();		// Parse expressions
	std::unique_ptr<ExprNode> parsePrimary();			// Parse primary expressions


	std::unique_ptr<VarDeclNode> parseVarDeclaration(const Token& token); // Parse variable declarations
	std::unique_ptr<ExprStatementNode> parseExprStatement(const Token& token); // Parse expression statements
	std::unique_ptr<IfStatementNode> parseIfStatement(const Token& token); // Parse if statements 
	std::unique_ptr<HtmlStmtRootNode> parseHtmlRootStatement(const Token& token); // Parse HTML root statements
	std::unique_ptr<HtmlBlockNode> parseHtmlBlockNode(); // Parse HTML block nodes
	std::unique_ptr<HtmlStmtNode> parseHtmlStatement(const Token& token); // Parse HTML statements
	std::unique_ptr<HtmlTextNode> parseHtmlTextNode(const Token& token); // Parse HTML text nodes
	std::unique_ptr<ReturnNode> parseReturnStatement(const Token& token); // Parse return statements
	std::unique_ptr<IncludeNode> parseIncludeStatement(const Token& token); // Parse include statements
	std::unique_ptr<BreakNode> parseBreakStatement(const Token& token); // Parse break statements
	std::unique_ptr<ContinueNode> parseContinueStatement(const Token& token); // Parse continue statements
	std::unique_ptr<PrintNode> parsePrintStatement(const Token& token); // Parse print statements
	std::unique_ptr<CommentNode> parseCommentNode(const Token& token); // Parse comment nodes

	// Loops
	std::unique_ptr<ForNode> parseForStatement(const Token& token); // Parse for loops
	std::unique_ptr<WhileNode> parseWhileStatement(const Token& token); // Parse while loops

	// Primarys
	std::unique_ptr<NativeFunctionCallNode> parseNativeFunctionCall(const Token& token); // Parse native function calls
	std::unique_ptr<ArrayLiteralNode> parseArrayLiteral(const Token& token); // Parse array literals
};

