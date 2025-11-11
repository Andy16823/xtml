#include "Parser.h"
#include <iostream>


Token& Parser::peek()
{
	if (m_pos >= m_tokens.size()) {
		return m_tokens.back();
	}
	return m_tokens[m_pos];
}

Token& Parser::peek(int offset)
{
	size_t targetPos = m_pos + static_cast<size_t>(offset);
	if (targetPos >= m_tokens.size()) {
		return m_tokens.back();
	}
	return m_tokens[targetPos];
}

Token& Parser::get()
{
	if (m_pos >= m_tokens.size()) {
		return m_tokens.back();
	}
	return m_tokens[m_pos++];
}

bool Parser::match(TokenType type, std::string value)
{
	const Token& t = peek();
	if (t.type == type && (value.empty() || t.value == value)) {
		m_pos++;
		return true;
	}
	return false;
}

std::unique_ptr<ASTNode> Parser::parse()
{
	// Since we are parsing a function definition at the top level
	auto functionNode = parseFunction();
	return functionNode;
}

std::unique_ptr<FunctionNode> Parser::parseFunction()
{
	// No parameters for now
	std::unique_ptr<FunctionNode> funcNode = std::make_unique<FunctionNode>();
	funcNode->body = parseBlock();
	return funcNode;
}

std::unique_ptr<BlockNode> Parser::parseBlock()
{
	std::unique_ptr<BlockNode> blockNode = std::make_unique<BlockNode>();
	while (!match(TokenType::EndOfFile, "")) {
		auto stmt = parseStatement();
		if (stmt != nullptr) {
			blockNode->add_child(std::move(stmt));
			continue;
		}
		// Add statement to block
	}
	return blockNode;
}

std::unique_ptr<StmtNode> Parser::parseStatement()
{
	auto& t = peek();
	if (t.type == TokenType::Keyword && t.value == "var") {
		return parseVarDeclaration(t);
	}

	if (t.type == TokenType::Identifier) {
		return parseExprStatement(t);
	}

	throw std::runtime_error("Error: Unknown statement starting with token '" + peek().value + "' at line " + std::to_string(peek().line) + ", column " + std::to_string(peek().column));
	return nullptr;
}

std::unique_ptr<ExprNode> Parser::parseExpression()
{
	// Binary expressions, literals, identifiers etc.
	auto left = parsePrimary();
	const Token& t = peek();
	if (t.type == TokenType::Operator) {
		std::string op = t.value;
		get(); // Consume operator
		auto right = parseExpression();
		auto binExpr = std::make_unique<BinaryExprNode>();
		binExpr->left = std::move(left);
		binExpr->right = std::move(right);
		binExpr->op = op;
		return binExpr;
	}

	return left;
}

std::unique_ptr<ExprNode> Parser::parsePrimary()
{
	const Token& t = get(); // Consume token and return it

	if (t.type == TokenType::IntegerLiteral) {
		return std::make_unique<IntegerLiteralNode>(std::stoi(t.value));
	}

	if (t.type == TokenType::StringLiteral) {
		return std::make_unique<StringLiteralNode>(t.value);
	}

	if (t.type == TokenType::FloatLiteral) {
		return std::make_unique<FloatLiteralNode>(std::stod(t.value));
	}

	if (t.type == TokenType::DoubleLiteral) {
		return std::make_unique<DoubleLiteralNode>(std::stod(t.value));
	}

	if (t.type == TokenType::Identifier && (t.value == "true" || t.value == "false")) {
		return std::make_unique<BoolLiteralNode>(t.value == "true");
	}

	if (t.type == TokenType::Identifier) {
		if (match(TokenType::Symbol, "("))
		{
			// Function call parsing can go here
		}
		else {
			return std::make_unique<VarExprNode>(t.value);
		}
	}

	throw std::runtime_error("Error: Unexpected token '" + t.value + "' at line " + std::to_string(t.line) + ", column " + std::to_string(t.column));
	return nullptr;
}

std::unique_ptr<VarDeclNode> Parser::parseVarDeclaration(const Token& token)
{
	get(); // Consume 'var' keyword
	auto vardecl = std::make_unique<VarDeclNode>();
	vardecl->name = get().value;
	if (match(TokenType::Operator, "=")) {
		auto expr = parseExpression();
		if (expr == nullptr) {
			std::cerr << "Error: Expected expression after '=' in variable declaration at line " << token.line << ", column " << token.column << ".\n";
			throw std::runtime_error("Error: Expected expression after '=' in variable declaration. At: " + std::to_string(token.line) + " " + std::to_string(token.column));
		}
		vardecl->expression = std::move(expr);
	}

	if(!match(TokenType::Symbol, ";")) {
		std::cerr << "Error: Missing semicolon at end of variable declaration at line " << token.line << ", column " << token.column << ".\n";
		throw std::runtime_error("Error: Missing semicolon at end of variable declaration.");
	}
	return vardecl;
}

std::unique_ptr<ExprStatementNode> Parser::parseExprStatement(const Token& token)
{
	auto expr = parseExpression();
	if (expr == nullptr) {
		std::cerr << "Error: Expected expression in expression statement at line " << token.line << ", column " << token.column << ".\n";
		throw std::runtime_error("Error: Expected expression in expression statement.");
	}
	auto exprStmt = std::make_unique<ExprStatementNode>();
	exprStmt->expression = std::move(expr);

	if(!match(TokenType::Symbol, ";")) {
		std::cerr << "Error: Missing semicolon at end of expression statement at line " << token.line << ", column " << token.column << ".\n";
		throw std::runtime_error("Error: Missing semicolon at end of expression statement.");
	}
	return exprStmt;
}
