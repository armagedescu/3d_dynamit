//! Expression tokenizer.
//!
//! Port of C++ expression_tokenizer.h

use crate::syntax_tree::SyntaxTree;

/// Token types
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TokenType {
    Unknown,
    Number,
    Space,
    Identifier,
    UnaryOperator,
    BinaryOperator,
    ExpressionBound, // ( or )
    Comma,
    End,
}

/// A token with its type and value
#[derive(Debug, Clone)]
pub struct Token {
    pub token_type: TokenType,
    pub value: String,
    pub position: usize,
}

impl Token {
    pub fn new(token_type: TokenType, value: String, position: usize) -> Self {
        Self { token_type, value, position }
    }
}

/// Result of tokenization attempt
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TokenResult {
    Accept,
    Reject,
    Empty,
    Finish,
    EndOfString,
}

/// Expression tokenizer
pub struct Tokenizer {
    binary_ops: SyntaxTree,
    unary_ops: SyntaxTree,
    parens: SyntaxTree,
}

impl Default for Tokenizer {
    fn default() -> Self {
        Self::new()
    }
}

impl Tokenizer {
    /// Create a new tokenizer with default operators
    pub fn new() -> Self {
        Self {
            binary_ops: SyntaxTree::merge(&["+", "-", "*", "**", "/", "%"]),
            unary_ops: SyntaxTree::merge(&["+", "-"]),
            parens: SyntaxTree::merge(&["(", ")"]),
        }
    }

    /// Tokenize an expression string
    pub fn tokenize(&self, source: &str) -> Vec<Token> {
        let mut tokens = Vec::new();
        let chars: Vec<char> = source.chars().collect();
        let mut pos = 0;

        self.tokenize_expression(&chars, &mut tokens, &mut pos);

        // Add end token or unknown for remaining
        if pos < chars.len() {
            let remaining: String = chars[pos..].iter().collect();
            tokens.push(Token::new(TokenType::Unknown, remaining, pos));
        } else {
            tokens.push(Token::new(TokenType::End, String::new(), pos));
        }

        tokens
    }

    fn tokenize_expression(&self, chars: &[char], tokens: &mut Vec<Token>, pos: &mut usize) {
        // Skip leading spaces
        self.eat_spaces(chars, tokens, pos);

        // Expect unary expression first
        let mut have_expr = false;
        if self.eat_unary_expression(chars, tokens, pos) == TokenResult::Accept {
            have_expr = true;
        }

        self.eat_spaces(chars, tokens, pos);

        if !have_expr {
            return;
        }

        // Continue with binary operators
        while *pos < chars.len() {
            // Try binary operator
            let op_result = self.eat_binary_operator(chars, tokens, pos);
            if op_result == TokenResult::Reject || op_result == TokenResult::Empty {
                break;
            }

            self.eat_spaces(chars, tokens, pos);

            if *pos >= chars.len() {
                break;
            }

            // Expect another unary expression
            let expr_result = self.eat_unary_expression(chars, tokens, pos);
            if expr_result == TokenResult::Reject || expr_result == TokenResult::Empty {
                break;
            }

            self.eat_spaces(chars, tokens, pos);

            if *pos >= chars.len() {
                break;
            }
        }
    }

    fn eat_spaces(&self, chars: &[char], tokens: &mut Vec<Token>, pos: &mut usize) -> TokenResult {
        let start = *pos;
        while *pos < chars.len() && chars[*pos].is_whitespace() {
            *pos += 1;
        }
        if *pos > start {
            let value: String = chars[start..*pos].iter().collect();
            tokens.push(Token::new(TokenType::Space, value, start));
            TokenResult::Accept
        } else {
            TokenResult::Empty
        }
    }

    fn eat_number(&self, chars: &[char], tokens: &mut Vec<Token>, pos: &mut usize) -> TokenResult {
        let start = *pos;
        let mut has_dot = false;
        let mut has_exp = false;
        let mut exp_sign_allowed = false;

        while *pos < chars.len() {
            let c = chars[*pos];

            if c.is_ascii_digit() {
                exp_sign_allowed = false;
                *pos += 1;
                continue;
            }

            if c == '.' && !has_dot && !has_exp {
                has_dot = true;
                exp_sign_allowed = false;
                *pos += 1;
                continue;
            }

            if (c == 'e' || c == 'E') && !has_exp && *pos > start {
                has_exp = true;
                exp_sign_allowed = true;
                *pos += 1;
                continue;
            }

            if (c == '+' || c == '-') && exp_sign_allowed {
                exp_sign_allowed = false;
                *pos += 1;
                continue;
            }

            break;
        }

        if *pos == start {
            return TokenResult::Empty;
        }

        let value: String = chars[start..*pos].iter().collect();

        // Validate: can't end with e, E, +, -, or just "."
        if let Some(last) = value.chars().last() {
            if last == 'e' || last == 'E' || last == '+' || last == '-' || last == '.' {
                *pos = start;
                return TokenResult::Reject;
            }
        }
        if value == "." {
            *pos = start;
            return TokenResult::Reject;
        }

        tokens.push(Token::new(TokenType::Number, value, start));
        TokenResult::Accept
    }

    fn eat_identifier(&self, chars: &[char], tokens: &mut Vec<Token>, pos: &mut usize) -> TokenResult {
        let start = *pos;

        if *pos >= chars.len() {
            return TokenResult::Empty;
        }

        let first = chars[*pos];
        if !first.is_alphabetic() && first != '_' {
            return TokenResult::Empty;
        }
        *pos += 1;

        while *pos < chars.len() {
            let c = chars[*pos];
            if c.is_alphanumeric() || c == '_' {
                *pos += 1;
            } else {
                break;
            }
        }

        let value: String = chars[start..*pos].iter().collect();
        tokens.push(Token::new(TokenType::Identifier, value, start));
        TokenResult::Accept
    }

    fn eat_binary_operator(&self, chars: &[char], tokens: &mut Vec<Token>, pos: &mut usize) -> TokenResult {
        let start = *pos;
        let mut value = String::new();

        while *pos < chars.len() {
            let c = chars[*pos];
            if self.binary_ops.in_alphabet(c) {
                value.push(c);
                *pos += 1;
            } else {
                break;
            }
        }

        if value.is_empty() {
            return TokenResult::Empty;
        }

        // Validate using syntax tree
        let result = self.binary_ops.find(&value);
        if !result.found && result.partial_finish == 0 {
            *pos = start;
            return TokenResult::Reject;
        }

        // Use the longest valid match
        let valid_len = if result.found && result.is_final {
            value.len()
        } else {
            result.partial_finish
        };

        if valid_len == 0 {
            *pos = start;
            return TokenResult::Reject;
        }

        *pos = start + valid_len;
        let valid_value: String = chars[start..*pos].iter().collect();
        tokens.push(Token::new(TokenType::BinaryOperator, valid_value, start));
        TokenResult::Accept
    }

    fn eat_unary_operator(&self, chars: &[char], tokens: &mut Vec<Token>, pos: &mut usize) -> TokenResult {
        if *pos >= chars.len() {
            return TokenResult::Empty;
        }

        let c = chars[*pos];
        if c == '+' || c == '-' {
            let start = *pos;
            *pos += 1;
            tokens.push(Token::new(TokenType::UnaryOperator, c.to_string(), start));
            TokenResult::Accept
        } else {
            TokenResult::Empty
        }
    }

    fn eat_paren(&self, chars: &[char], tokens: &mut Vec<Token>, pos: &mut usize, expected: Option<char>) -> TokenResult {
        if *pos >= chars.len() {
            return TokenResult::Empty;
        }

        let c = chars[*pos];
        if c != '(' && c != ')' {
            return TokenResult::Empty;
        }

        if let Some(exp) = expected {
            if c != exp {
                return TokenResult::Reject;
            }
        }

        let start = *pos;
        *pos += 1;
        tokens.push(Token::new(TokenType::ExpressionBound, c.to_string(), start));
        TokenResult::Accept
    }

    fn eat_comma(&self, chars: &[char], tokens: &mut Vec<Token>, pos: &mut usize) -> TokenResult {
        if *pos >= chars.len() || chars[*pos] != ',' {
            return TokenResult::Empty;
        }

        let start = *pos;
        *pos += 1;
        tokens.push(Token::new(TokenType::Comma, ",".to_string(), start));
        TokenResult::Accept
    }

    fn eat_grouping_paren(&self, chars: &[char], tokens: &mut Vec<Token>, pos: &mut usize) -> TokenResult {
        if *pos >= chars.len() || chars[*pos] != '(' {
            return TokenResult::Empty;
        }

        // Opening paren
        if self.eat_paren(chars, tokens, pos, Some('(')) != TokenResult::Accept {
            return TokenResult::Empty;
        }

        // Inner expression
        let expr_start = *pos;
        self.tokenize_expression(chars, tokens, pos);
        if *pos == expr_start {
            return TokenResult::Reject;
        }

        // Closing paren
        if self.eat_paren(chars, tokens, pos, Some(')')) != TokenResult::Accept {
            return TokenResult::Reject;
        }

        TokenResult::Accept
    }

    fn eat_function_call(&self, chars: &[char], tokens: &mut Vec<Token>, pos: &mut usize) -> TokenResult {
        let start = *pos;
        let token_start = tokens.len();

        // Identifier
        if self.eat_identifier(chars, tokens, pos) != TokenResult::Accept {
            return TokenResult::Empty;
        }

        self.eat_spaces(chars, tokens, pos);

        // Must be followed by '('
        if *pos >= chars.len() || chars[*pos] != '(' {
            // Not a function call, revert
            *pos = start;
            tokens.truncate(token_start);
            return TokenResult::Empty;
        }

        // Opening paren
        if self.eat_paren(chars, tokens, pos, Some('(')) != TokenResult::Accept {
            *pos = start;
            tokens.truncate(token_start);
            return TokenResult::Empty;
        }

        self.eat_spaces(chars, tokens, pos);

        // Check for empty arg list
        if *pos < chars.len() && chars[*pos] == ')' {
            if self.eat_paren(chars, tokens, pos, Some(')')) != TokenResult::Accept {
                return TokenResult::Reject;
            }
            return TokenResult::Accept;
        }

        // Parse arguments
        let arg_start = *pos;
        self.tokenize_expression(chars, tokens, pos);
        if *pos == arg_start {
            return TokenResult::Reject;
        }

        // More arguments?
        loop {
            self.eat_spaces(chars, tokens, pos);

            if self.eat_comma(chars, tokens, pos) != TokenResult::Accept {
                break;
            }

            self.eat_spaces(chars, tokens, pos);

            let arg_start = *pos;
            self.tokenize_expression(chars, tokens, pos);
            if *pos == arg_start {
                return TokenResult::Reject;
            }
        }

        // Closing paren
        if self.eat_paren(chars, tokens, pos, Some(')')) != TokenResult::Accept {
            return TokenResult::Reject;
        }

        TokenResult::Accept
    }

    fn eat_primary(&self, chars: &[char], tokens: &mut Vec<Token>, pos: &mut usize) -> TokenResult {
        // Try grouping parentheses
        let result = self.eat_grouping_paren(chars, tokens, pos);
        if result == TokenResult::Accept {
            return TokenResult::Accept;
        }
        if result == TokenResult::Reject {
            return TokenResult::Reject;
        }

        // Try number
        let result = self.eat_number(chars, tokens, pos);
        if result == TokenResult::Accept {
            return TokenResult::Accept;
        }

        // Try function call
        let result = self.eat_function_call(chars, tokens, pos);
        if result == TokenResult::Accept {
            return TokenResult::Accept;
        }
        if result == TokenResult::Reject {
            return TokenResult::Reject;
        }

        // Try identifier (variable or constant)
        let result = self.eat_identifier(chars, tokens, pos);
        if result == TokenResult::Accept {
            return TokenResult::Accept;
        }

        TokenResult::Empty
    }

    fn eat_unary_expression(&self, chars: &[char], tokens: &mut Vec<Token>, pos: &mut usize) -> TokenResult {
        let start = *pos;

        // Optional unary operator
        let had_unary = self.eat_unary_operator(chars, tokens, pos) == TokenResult::Accept;

        self.eat_spaces(chars, tokens, pos);

        // Primary expression
        let result = self.eat_primary(chars, tokens, pos);
        if result == TokenResult::Reject {
            return TokenResult::Reject;
        }
        if result == TokenResult::Empty {
            if had_unary {
                return TokenResult::Reject;
            }
            *pos = start;
            return TokenResult::Empty;
        }

        TokenResult::Accept
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_tokenize_number() {
        let tokenizer = Tokenizer::new();
        let tokens = tokenizer.tokenize("42");
        assert_eq!(tokens.len(), 2); // number + end
        assert_eq!(tokens[0].token_type, TokenType::Number);
        assert_eq!(tokens[0].value, "42");
    }

    #[test]
    fn test_tokenize_float() {
        let tokenizer = Tokenizer::new();
        let tokens = tokenizer.tokenize("3.14159");
        assert_eq!(tokens[0].token_type, TokenType::Number);
        assert_eq!(tokens[0].value, "3.14159");
    }

    #[test]
    fn test_tokenize_scientific() {
        let tokenizer = Tokenizer::new();
        let tokens = tokenizer.tokenize("1.5e-10");
        assert_eq!(tokens[0].token_type, TokenType::Number);
        assert_eq!(tokens[0].value, "1.5e-10");
    }

    #[test]
    fn test_tokenize_expression() {
        let tokenizer = Tokenizer::new();
        let tokens = tokenizer.tokenize("1 + 2 * 3");

        let non_space: Vec<_> = tokens.iter()
            .filter(|t| t.token_type != TokenType::Space && t.token_type != TokenType::End)
            .collect();

        assert_eq!(non_space.len(), 5);
        assert_eq!(non_space[0].value, "1");
        assert_eq!(non_space[1].value, "+");
        assert_eq!(non_space[2].value, "2");
        assert_eq!(non_space[3].value, "*");
        assert_eq!(non_space[4].value, "3");
    }

    #[test]
    fn test_tokenize_function() {
        let tokenizer = Tokenizer::new();
        let tokens = tokenizer.tokenize("sin(x)");

        let non_space: Vec<_> = tokens.iter()
            .filter(|t| t.token_type != TokenType::Space && t.token_type != TokenType::End)
            .collect();

        assert_eq!(non_space[0].token_type, TokenType::Identifier);
        assert_eq!(non_space[0].value, "sin");
        assert_eq!(non_space[1].token_type, TokenType::ExpressionBound);
        assert_eq!(non_space[1].value, "(");
        assert_eq!(non_space[2].token_type, TokenType::Identifier);
        assert_eq!(non_space[2].value, "x");
        assert_eq!(non_space[3].token_type, TokenType::ExpressionBound);
        assert_eq!(non_space[3].value, ")");
    }

    #[test]
    fn test_tokenize_power() {
        let tokenizer = Tokenizer::new();
        let tokens = tokenizer.tokenize("x ** 2");

        let non_space: Vec<_> = tokens.iter()
            .filter(|t| t.token_type != TokenType::Space && t.token_type != TokenType::End)
            .collect();

        assert_eq!(non_space[1].token_type, TokenType::BinaryOperator);
        assert_eq!(non_space[1].value, "**");
    }

    #[test]
    fn test_tokenize_unary_minus() {
        let tokenizer = Tokenizer::new();
        let tokens = tokenizer.tokenize("-x");

        let non_space: Vec<_> = tokens.iter()
            .filter(|t| t.token_type != TokenType::Space && t.token_type != TokenType::End)
            .collect();

        assert_eq!(non_space[0].token_type, TokenType::UnaryOperator);
        assert_eq!(non_space[0].value, "-");
        assert_eq!(non_space[1].token_type, TokenType::Identifier);
        assert_eq!(non_space[1].value, "x");
    }
}
