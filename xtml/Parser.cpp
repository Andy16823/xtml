#include "Parser.h"
#include <iostream>
#include "Utils.h"
#include <sstream>


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

bool Parser::match(TokenType type, std::string value, size_t range)
{
	std::string tvalues;
	for (size_t i = 0; i < range; i++) {
		const Token& t = peek(i); 
		tvalues += t.value;
		if (t.type != type) {
			return false;
		}
	}
	if(tvalues != value) {
		return false;
	}
	m_pos += range;
	return true;
}

std::unique_ptr<XtmlBlockNode> Parser::parse()
{
	// Toplevel parse block since we dont start with functions
	auto xtmlBlock = std::make_unique<XtmlBlockNode>();
	xtmlBlock->body = parseBlock();
	return xtmlBlock;
}

std::unique_ptr<FunctionDeclNode> Parser::parseFunction()
{
	get(); // Consume 'function' keyword
	auto function = std::make_unique<FunctionNode>();
	function->name = get().value;

	if (!match(TokenType::Symbol, "(")) {
		Utils::throwErr("Error: Expected '(' after function name at line " + std::to_string(peek().line) + ", column " + std::to_string(peek().column) + ".");
	}

	// Parse parameters (not implemented yet)
	while (true) {
		// Early EOF check
		if(peek().type == TokenType::EndOfFile) {
			Utils::throwErr("Error: Unexpected end of file while parsing function parameters at line " + std::to_string(peek().line) + ", column " + std::to_string(peek().column) + ".");
		}

		// Parse the parameter expression
		auto param = parseExpression();
		if(param == nullptr) {
			Utils::throwErr("Error: Expected parameter expression in function parameter list at line " + std::to_string(peek().line) + ", column " + std::to_string(peek().column) + ".");
		}
		function->arguments.push_back(std::move(param));
		
		if (!match(TokenType::Symbol, ",")) {
			if(!match(TokenType::Symbol, ")")) {
				Utils::throwErr("Error: Expected ',' or ')' in function parameter list at line " + std::to_string(peek().line) + ", column " + std::to_string(peek().column) + ".");
			}
			else {
				break;
			}
		}
	}

	// Parse function body
	function->body = parseBracketBlock();
	return std::make_unique<FunctionDeclNode>(std::move(function));
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

std::unique_ptr<BlockNode> Parser::parseBracketBlock()
{
	if(!match(TokenType::Symbol, "{")) {
		Utils::throwErr("Error: Expected '{' to start block at line " + std::to_string(peek().line) + ", column " + std::to_string(peek().column) + ".");
	}
	auto blockNode = std::make_unique<BlockNode>();
	while (!match(TokenType::Symbol, "}")) {
		if(peek().type == TokenType::EndOfFile) {
			Utils::throwErr("Error: Unexpected end of file while parsing block starting at line " + std::to_string(peek().line) + ", column " + std::to_string(peek().column) + ".");
		}
		auto stmt = parseStatement();
		if (stmt != nullptr) {
			blockNode->add_child(std::move(stmt));
			continue;
		}
	}
	return blockNode;
}

std::unique_ptr<StmtNode> Parser::parseStatement()
{
	auto& t = peek();
	if (t.type == TokenType::Keyword && t.value == "var") {
		return parseVarDeclaration(t);
	}

	if (t.type == TokenType::Keyword && t.value == "if") {
		return parseIfStatement(t);
	}

	if (t.type == TokenType::Keyword && t.value == "html") {
		return parseHtmlRootStatement(t);
	}

	if (t.type == TokenType::Keyword && t.value == "for") {
		return parseForStatement(t);
	}

	if (t.type == TokenType::Keyword && t.value == "while") {
		return parseWhileStatement(t);
	}

	if (t.type == TokenType::Operator && t.value == "<") {
		return parseHtmlStatement(t);
	}

	if (t.type == TokenType::Keyword && t.value == "function") {
		return parseFunction();
	}

	if (t.type == TokenType::Keyword && t.value == "return") {
		return parseReturnStatement(t);
	}

	if (t.type == TokenType::Keyword && t.value == "include") {
		return parseIncludeStatement(t);
	}

	if (t.type == TokenType::Keyword && t.value == "break") {
		return parseBreakStatement(t);
	}

	if (t.type == TokenType::Keyword && t.value == "continue") {
		return parseContinueStatement(t);
	}

	if (t.type == TokenType::Keyword && t.value == "print") {
		return parsePrintStatement(t);
	}

	if (t.type == TokenType::Operator && t.value == "//") {
		return parseCommentNode(t);
	}

	if (t.type == TokenType::Identifier) {
		return parseExprStatement(t);
	}

	Utils::throwErr("Error: Unable to parse statement at line " + std::to_string(t.line) + ", column " + std::to_string(t.column));
}

std::unique_ptr<ExprNode> Parser::parseExpression()
{
	auto token = peek();

	// Check for unary prefix operators
	if (token.type == TokenType::Operator && (token.value == "++" || token.value == "--" || token.value == "+" || token.value == "-")) {
		get(); // Consume operator
		auto unaryExpr = std::make_unique<UnaryExprNode>();
		unaryExpr->op = token.value;
		unaryExpr->isPrefix = true;
		unaryExpr->operand = parsePrimary();
		return unaryExpr;
	}

	// If no unary prefix, parse primary expression
	auto left = parsePrimary();
	const Token& t = peek();
	if (t.type == TokenType::Operator) {

		// Check for unary postfix operators
		if (t.value == "++" || t.value == "--") {
			get(); // Consume operator
			auto unaryExpr = std::make_unique<UnaryExprNode>();
			unaryExpr->op = t.value;
			unaryExpr->isPrefix = false;
			unaryExpr->operand = std::move(left);
			return unaryExpr;
		}

		// If not postfix, it must be binary
		std::string op = t.value;
		get();
		auto right = parseExpression();
		auto binExpr = std::make_unique<BinaryExprNode>();
		binExpr->left = std::move(left);
		binExpr->right = std::move(right);
		binExpr->op = op;
		return binExpr;
	}

	// If no operators, return the primary expression
	return left;
}

std::unique_ptr<ExprNode> Parser::parsePrimary()
{
	const Token& t = get(); // Consume token and return it

	// Case 10
	if (t.type == TokenType::IntegerLiteral) {
		return std::make_unique<IntegerLiteralNode>(std::stoi(t.value));
	}

	// Case "Hello"
	if (t.type == TokenType::StringLiteral) {
		return std::make_unique<StringLiteralNode>(t.value);
	}

	// Case 3.14f
	if (t.type == TokenType::FloatLiteral) {
		return std::make_unique<FloatLiteralNode>(std::stod(t.value));
	}

	// Case 2.71828
	if (t.type == TokenType::DoubleLiteral) {
		return std::make_unique<DoubleLiteralNode>(std::stod(t.value));
	}

	if (t.type == TokenType::Symbol && t.value == "[") {
		return this->parseArrayLiteral(t);
	}

	// Case true/false
	if (t.type == TokenType::Identifier && (t.value == "true" || t.value == "false")) {
		return std::make_unique<BoolLiteralNode>(t.value == "true");
	}

	// Case native function call native std::foo(10)
	if (t.type == TokenType::Keyword && t.value == "native") {
		return this->parseNativeFunctionCall(t);
	}

	// Case variable or function call
	if (t.type == TokenType::Identifier) {
		if (match(TokenType::Symbol, "("))
		{
			auto funcCall = std::make_unique<FunctionCallNode>();
			funcCall->functionName = t.value;
			while (true) {
				auto expr = parseExpression();
				if (expr != nullptr) {
					funcCall->arguments.push_back(std::move(expr));
				}
				if (!match(TokenType::Symbol, ",")) {
					if (!match(TokenType::Symbol, ")")) {
						Utils::throwErr("Error");
					}
					break;
				}
			}
			return funcCall;
		}
		else {
			return std::make_unique<VarExprNode>(t.value);
		}
	}

	// Case (expression)
	if (t.type == TokenType::Symbol && t.value == "(") {
		auto expr = parseExpression();
		if (!match(TokenType::Symbol, ")")) {
			Utils::throwErr("Error: Expected ')' after expression at line " + std::to_string(t.line) + ", column " + std::to_string(t.column) + ".");
		}
		return expr;
	}

	if (t.type == TokenType::Keyword && t.value == "expr") {
		if(!match(TokenType::Symbol, "{")) {
			Utils::throwErr("Error: Expected '{' after 'expr' at line " + std::to_string(t.line) + ", column " + std::to_string(t.column) + ".");
		}
		auto stmtExpr = std::make_unique<StmtExprNode>();
		auto stmt = parseStatement();
		if (stmt == nullptr) {
			Utils::throwErr("Error: Expected statement after 'expr' at line " + std::to_string(t.line) + ", column " + std::to_string(t.column) + ".");
		}
		if(!match(TokenType::Symbol, "}")) {
			Utils::throwErr("Error: Expected '}' after statement in 'expr' at line " + std::to_string(t.line) + ", column " + std::to_string(t.column) + ".");
		}
		stmtExpr->statement = std::move(stmt);
		return stmtExpr;
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

std::unique_ptr<IfStatementNode> Parser::parseIfStatement(const Token& token)
{
	get(); // Consume 'if' keyword
	auto ifStmt = std::make_unique<IfStatementNode>();
	if (!match(TokenType::Symbol, "(")) {
		Utils::throwErr("Error: Expected '(' after 'if' at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	ifStmt->condition = parseExpression();
	if (!match(TokenType::Symbol, ")")) {
		Utils::throwErr("Error: Expected ')' after if condition at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	ifStmt->body = parseBracketBlock();

	// Check for else if and else clauses
	while (match(TokenType::Keyword, "else")) {
		// Eearly end of file check
		if(peek().type == TokenType::EndOfFile) {
			Utils::throwErr("Error: Unexpected end of file after 'else' at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
		}

		if (match(TokenType::Keyword, "if")) {
			// Else if clause
			auto elseifstmt = std::make_unique<IfStatementNode>();

			// Check for condition
			if (!match(TokenType::Symbol, "(")) {
				Utils::throwErr("Error: Expected '(' after 'else if' at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
			}

			// Parse condition
			elseifstmt->condition = parseExpression();

			// Expect closing parenthesis
			if (!match(TokenType::Symbol, ")")) {
				Utils::throwErr("Error: Expected ')' after else if condition at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
			}

			// Parse body for else if and add to ifStmt
			elseifstmt->body = parseBracketBlock();
			ifStmt->elseIfs.push_back(std::move(elseifstmt));
		}
		else {
			// Else clause - parse body and assign to elseBody
			ifStmt->elseBody = parseBracketBlock();
		}
	}

	return ifStmt;
}

std::unique_ptr<HtmlStmtRootNode> Parser::parseHtmlRootStatement(const Token& token)
{
	get(); // Consume 'html' keyword
	auto htmlRoot = std::make_unique<HtmlStmtRootNode>();
	htmlRoot->body = parseHtmlBlockNode();
	return htmlRoot;
}

std::unique_ptr<HtmlBlockNode> Parser::parseHtmlBlockNode()
{
	// Check for opening {
	if(!match(TokenType::Symbol, "{")) {
		Utils::throwErr("Error: Expected '{' to start HTML block at line " + std::to_string(peek().line) + ", column " + std::to_string(peek().column) + ".");
	}

	// Create HTML block node
	auto htmlBlock = std::make_unique<HtmlBlockNode>();
	
	// Parse until closing }
	while (!match(TokenType::Symbol, "}")) {
		const auto& t = peek();
		if(peek().type == TokenType::EndOfFile) {
			Utils::throwErr("Error: Unexpected end of file while parsing HTML block at line " + std::to_string(t.line) + ", column " + std::to_string(t.column) + ".");
		}

		// Check for HTML statement
		if(t.type == TokenType::Operator && t.value == "<") {
			auto htmlStmt = parseHtmlStatement(t);
			htmlBlock->add_child(std::move(htmlStmt));
			continue;
		}

		// Check for string literal
		if (t.type == TokenType::StringLiteral) {
			auto htmlTextNode = std::make_unique<HtmlTextNode>(t.value);
			htmlBlock->add_child(std::move(htmlTextNode));
			get();
			continue;
		}
		
		// Otherwise, error
		Utils::throwErr("Error: Unexpected token '" + t.value + "' in HTML block at line " + std::to_string(t.line) + ", column " + std::to_string(t.column) + ".");
	}
	return htmlBlock;
}

std::unique_ptr<HtmlStmtNode> Parser::parseHtmlStatement(const Token& token)
{
	get(); // Consume <
	auto htmlStmt = std::make_unique<HtmlStmtNode>();
	htmlStmt->tagName = get().value; // Tag name
	while (true) {
		// End of file check
		if(peek().type == TokenType::EndOfFile) {
			Utils::throwErr("Error: Unexpected end of file while parsing HTML tag <" + htmlStmt->tagName + "> at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
		}

		// First exit condition tag end
		if(match(TokenType::Operator, ">")) {
			break;
		}
		// Second exit condition self-closing tag
		if (match(TokenType::Operator, "/>")) {
			htmlStmt->selfClosing = true;
			break;
		}

		// Parse attributes
		std::string attrName = get().value;
		if (!match(TokenType::Operator, "=")) {
			Utils::throwErr("Error: Expected '=' after attribute name '" + attrName + "' in HTML tag at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
		}
		std::string attrValue = get().value;
		htmlStmt->attributes[attrName] = Utils::trimQuotes(attrValue);
	}

	// If not self-closing, parse content and closing tag
	if (!htmlStmt->selfClosing) {
		// Parse content until closing tag
		std::string content;
		while (!match(TokenType::Operator , "</")) {
			if(peek().type == TokenType::EndOfFile) {
				Utils::throwErr("Error: Unexpected end of file while parsing content of <" + htmlStmt->tagName + "> at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
			}

			// Check for nested tags
			if (peek().type == TokenType::Operator && peek().value == "<") 
			{
				auto nestedTag = parseHtmlStatement(token);
				htmlStmt->children.push_back(std::move(nestedTag));
				continue;
			}

			auto& token = get();
			auto htmlTextNode = parseHtmlTextNode(token);
			htmlStmt->children.push_back(std::move(htmlTextNode));
		}

		// Expect closing tag since the while loop exited and removed </
		std::string closingTagName = get().value;
		if(htmlStmt->tagName != closingTagName) {
			Utils::throwErr("Error: Mismatched closing tag </" + closingTagName + "> for <" + htmlStmt->tagName + "> at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
		}
		if (!match(TokenType::Operator, ">")) {
			Utils::throwErr("Error: Expected '>' at end of closing tag </" + closingTagName + "> at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
		}
		return htmlStmt;
	}

	return htmlStmt; // self-closing tag
}

std::unique_ptr<HtmlTextNode> Parser::parseHtmlTextNode(const Token& token)
{
	std::unique_ptr<HtmlTextNode> textNode = std::make_unique<HtmlTextNode>();
	std::ostringstream content;

	// Add current token value
	content << token.value << " ";

	// Continue until next token is an operator < or </
	auto next = peek();
	while (next.type != TokenType::Operator && (next.value != "<" || next.value != "</")) {
		if(next.type == TokenType::EndOfFile) {
			Utils::throwErr("Error: Unexpected end of file while parsing HTML text node at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
		}
		content << get().value << " ";
		next = peek();
	}

	// Set content and return
	textNode->content = Utils::trim(content.str());
	return textNode;
}

std::unique_ptr<ReturnNode> Parser::parseReturnStatement(const Token& token)
{
	get(); // Consume 'return' keyword
	auto returnNode = std::make_unique<ReturnNode>();
	returnNode->expression = parseExpression();
	if(!match(TokenType::Symbol, ";")) {
		Utils::throwErr("Error: Expected ';' after return statement at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	return returnNode;
}

std::unique_ptr<IncludeNode> Parser::parseIncludeStatement(const Token& token)
{
	get(); // Consume 'include' keyword
	auto includeNode = std::make_unique<IncludeNode>();
	if(peek().type != TokenType::StringLiteral) {
		Utils::throwErr("Error: Expected string literal for include path at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	includeNode->includePath = Utils::trimQuotes(get().value);
	if (!match(TokenType::Symbol, ";")) {
		Utils::throwErr("Error: Expected ';' after include statement at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	return includeNode;
}

std::unique_ptr<BreakNode> Parser::parseBreakStatement(const Token& token)
{
	get(); // Consume 'break' keyword
	auto breakNode = std::make_unique<BreakNode>();
	if (!match(TokenType::Symbol, ";")) {
		Utils::throwErr("Error: Expected ';' after break statement at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	return breakNode;
}

std::unique_ptr<ContinueNode> Parser::parseContinueStatement(const Token& token)
{
	get(); // Consume 'continue' keyword
	auto continueNode = std::make_unique<ContinueNode>();
	if (!match(TokenType::Symbol, ";")) {
		Utils::throwErr("Error: Expected ';' after continue statement at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	return continueNode;
}

std::unique_ptr<PrintNode> Parser::parsePrintStatement(const Token& token)
{
	get(); // Consume 'print' keyword
	if (!match(TokenType::Symbol, "(")) {
		Utils::throwErr("Error: Expected '(' after 'print' at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	auto printNode = std::make_unique<PrintNode>();
	printNode->expression = parseExpression();
	if (!match(TokenType::Symbol, ")")) {
		Utils::throwErr("Error: Expected ')' after print expression at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	if (!match(TokenType::Symbol, ";")) {
		Utils::throwErr("Error: Expected ';' after print statement at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	return printNode;
}

std::unique_ptr<CommentNode> Parser::parseCommentNode(const Token& token)
{
	get(); // Consume comment token //
	auto commentNode = std::make_unique<CommentNode>();
	while (peek().type != TokenType::EndOfFile && peek().line == token.line) {
		commentNode->content += get().value + " ";
	}
	commentNode->content = Utils::trim(commentNode->content);
	return commentNode;
}

std::unique_ptr<ForNode> Parser::parseForStatement(const Token& token)
{
	get(); // Consume 'for' keyword
	auto forNode = std::make_unique<ForNode>();
	if (!match(TokenType::Symbol, "(")) {
		Utils::throwErr("Error: Expected '(' after 'for' at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	// Parse initialization No semicolon removal here, handled in parseStatement
	forNode->init = parseStatement();
	// Parse condition
	forNode->condition = parseExpression();
	if(!match(TokenType::Symbol, ";")) {
		Utils::throwErr("Error: Expected ';' after for loop condition at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	// Parse increment
	forNode->increment = parseExpression();
	if (!match(TokenType::Symbol, ")")) {
		Utils::throwErr("Error: Expected ')' after for loop clauses at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	forNode->body = parseBracketBlock();
	return forNode;
}

std::unique_ptr<WhileNode> Parser::parseWhileStatement(const Token& token)
{
	get(); // Consume 'while' keyword
	auto whileNode = std::make_unique<WhileNode>();
	if (!match(TokenType::Symbol, "(")) {
		Utils::throwErr("Error: Expected '(' after 'while' at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	whileNode->condition = parseExpression();
	if (!match(TokenType::Symbol, ")")) {
		Utils::throwErr("Error: Expected ')' after while loop condition at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	whileNode->body = parseBracketBlock();
	return whileNode;
}

std::unique_ptr<NativeFunctionCallNode> Parser::parseNativeFunctionCall(const Token& token)
{
	auto nativeCall = std::make_unique<NativeFunctionCallNode>();
	nativeCall->namespaceName = get().value; // Get namespace
	if (!match(TokenType::Symbol, "::", 2)) {
		Utils::throwErr("Error: Expected '::' after native namespace at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	nativeCall->functionName = get().value; // Get function name
	if (!match(TokenType::Symbol, "(")) {
		Utils::throwErr("Error: Expected '(' after native function name at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
	}
	while (true) {
		// Early EOF check
		if (peek().type == TokenType::EndOfFile) {
			Utils::throwErr("Error: Unexpected end of file while parsing native function arguments at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
		}

		// Parse the argument expression
		auto expr = parseExpression();
		if (expr != nullptr) {
			nativeCall->arguments.push_back(std::move(expr));
		}

		// Check for ',' or ')'
		if (!match(TokenType::Symbol, ",")) {
			if (!match(TokenType::Symbol, ")")) {
				Utils::throwErr("Error: Expected ')' after native function arguments at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
			}
			break;
		}
	}
	return nativeCall;
}

std::unique_ptr<ArrayLiteralNode> Parser::parseArrayLiteral(const Token& token)
{
	auto arrayLiteral = std::make_unique<ArrayLiteralNode>();
	while (true) {
		// Early EOF check
		if(peek().type == TokenType::EndOfFile) {
			Utils::throwErr("Error: Unexpected end of file while parsing array literal at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
		}

		// Parse the element expression
		auto expr = parseExpression();
		if (expr != nullptr) {
			arrayLiteral->elements.push_back(std::move(expr));
		}

		// Check for ',' or ']'
		if (!match(TokenType::Symbol, ",")) {
			if (!match(TokenType::Symbol, "]")) {
				Utils::throwErr("Error: Expected ',' or ']' in array literal at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ".");
			}
			else {
				break;
			}
		}
	}
	return arrayLiteral;
}
