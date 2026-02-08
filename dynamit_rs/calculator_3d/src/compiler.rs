//! Expression compiler - compiles tokens into an evaluable AST.
//!
//! Port of C++ expression_compiler.h

use std::collections::HashMap;
use std::f64::consts::{E, PI, TAU};

use crate::tokenizer::{Token, TokenType, Tokenizer};

/// Binary operators
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BinaryOperator {
    Plus,
    Minus,
    Multiply,
    Divide,
    Power,
}

impl BinaryOperator {
    /// Get operator precedence (higher = binds tighter)
    pub fn precedence(self) -> u8 {
        match self {
            BinaryOperator::Plus | BinaryOperator::Minus => 1,
            BinaryOperator::Multiply | BinaryOperator::Divide => 2,
            BinaryOperator::Power => 4,
        }
    }

    /// Check if operator is right-associative
    pub fn is_right_associative(self) -> bool {
        matches!(self, BinaryOperator::Power)
    }
}

/// Unary operators
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum UnaryOperator {
    Plus,
    Minus,
}

/// Function pointer types
type UnaryFn = fn(f64) -> f64;
type BinaryFn = fn(f64, f64) -> f64;

/// Function entry in the registry
#[derive(Clone)]
struct FunctionEntry {
    arity: u8,
    unary_fn: Option<UnaryFn>,
    binary_fn: Option<BinaryFn>,
}

/// Built-in function registry
struct FunctionRegistry {
    functions: HashMap<String, FunctionEntry>,
}

impl FunctionRegistry {
    fn new() -> Self {
        let mut functions = HashMap::new();

        // Unary functions
        let unary_fns: &[(&str, UnaryFn)] = &[
            ("sin", f64::sin),
            ("cos", f64::cos),
            ("tan", f64::tan),
            ("asin", f64::asin),
            ("acos", f64::acos),
            ("atan", f64::atan),
            ("sinh", f64::sinh),
            ("cosh", f64::cosh),
            ("tanh", f64::tanh),
            ("sqrt", f64::sqrt),
            ("exp", f64::exp),
            ("log", f64::ln),
            ("ln", f64::ln),
            ("log10", f64::log10),
            ("log2", f64::log2),
            ("abs", f64::abs),
            ("floor", f64::floor),
            ("ceil", f64::ceil),
            ("round", f64::round),
            ("signum", f64::signum),
        ];

        for (name, func) in unary_fns {
            functions.insert(
                name.to_lowercase(),
                FunctionEntry {
                    arity: 1,
                    unary_fn: Some(*func),
                    binary_fn: None,
                },
            );
        }

        // Binary functions
        let binary_fns: &[(&str, BinaryFn)] = &[
            ("pow", f64::powf),
            ("atan2", f64::atan2),
            ("min", f64::min),
            ("max", f64::max),
            ("hypot", f64::hypot),
        ];

        for (name, func) in binary_fns {
            functions.insert(
                name.to_lowercase(),
                FunctionEntry {
                    arity: 2,
                    unary_fn: None,
                    binary_fn: Some(*func),
                },
            );
        }

        Self { functions }
    }

    fn get(&self, name: &str) -> Option<&FunctionEntry> {
        self.functions.get(&name.to_lowercase())
    }
}

/// Built-in constant registry
struct ConstantRegistry {
    constants: HashMap<String, f64>,
}

impl ConstantRegistry {
    fn new() -> Self {
        let mut constants = HashMap::new();

        constants.insert("pi".to_string(), PI);
        constants.insert("m_pi".to_string(), PI);
        constants.insert("e".to_string(), E);
        constants.insert("m_e".to_string(), E);
        constants.insert("tau".to_string(), TAU);
        constants.insert("m_pi_2".to_string(), PI / 2.0);
        constants.insert("m_pi_4".to_string(), PI / 4.0);
        constants.insert("m_1_pi".to_string(), 1.0 / PI);
        constants.insert("m_2_pi".to_string(), 2.0 / PI);
        constants.insert("m_sqrt2".to_string(), std::f64::consts::SQRT_2);
        constants.insert("m_ln2".to_string(), std::f64::consts::LN_2);
        constants.insert("m_ln10".to_string(), std::f64::consts::LN_10);

        Self { constants }
    }

    fn get(&self, name: &str) -> Option<f64> {
        self.constants.get(&name.to_lowercase()).copied()
    }
}

/// Expression context - holds variable bindings
#[derive(Default)]
pub struct ExpressionContext {
    bindings: HashMap<String, *mut f64>,
}

impl ExpressionContext {
    pub fn new() -> Self {
        Self::default()
    }

    /// Bind a variable name to a mutable reference
    ///
    /// # Safety
    /// The caller must ensure the pointer remains valid for the lifetime of the context
    pub unsafe fn bind_ptr(&mut self, name: &str, ptr: *mut f64) {
        self.bindings.insert(name.to_lowercase(), ptr);
    }

    /// Get the value of a bound variable
    pub fn get(&self, name: &str) -> Option<f64> {
        self.bindings.get(&name.to_lowercase()).map(|&ptr| unsafe { *ptr })
    }

    /// Check if a variable is bound
    pub fn has(&self, name: &str) -> bool {
        self.bindings.contains_key(&name.to_lowercase())
    }

    /// Clear all bindings
    pub fn clear(&mut self) {
        self.bindings.clear();
    }
}

/// Expression node types
enum ExprNode {
    Constant(f64),
    Variable(String),
    Unary {
        op: UnaryOperator,
        operand: Box<ExprNode>,
    },
    Binary {
        op: BinaryOperator,
        left: Box<ExprNode>,
        right: Box<ExprNode>,
    },
    UnaryFunc {
        func: UnaryFn,
        arg: Box<ExprNode>,
    },
    BinaryFunc {
        func: BinaryFn,
        arg1: Box<ExprNode>,
        arg2: Box<ExprNode>,
    },
    SubExpr {
        inner: Box<ExprNode>,
    },
}

impl ExprNode {
    fn eval(&self, ctx: &ExpressionContext) -> f64 {
        match self {
            ExprNode::Constant(v) => *v,
            ExprNode::Variable(name) => ctx.get(name).unwrap_or(0.0),
            ExprNode::Unary { op, operand } => {
                let v = operand.eval(ctx);
                match op {
                    UnaryOperator::Plus => v,
                    UnaryOperator::Minus => -v,
                }
            }
            ExprNode::Binary { op, left, right } => {
                let l = left.eval(ctx);
                let r = right.eval(ctx);
                match op {
                    BinaryOperator::Plus => l + r,
                    BinaryOperator::Minus => l - r,
                    BinaryOperator::Multiply => l * r,
                    BinaryOperator::Divide => l / r,
                    BinaryOperator::Power => l.powf(r),
                }
            }
            ExprNode::UnaryFunc { func, arg } => func(arg.eval(ctx)),
            ExprNode::BinaryFunc { func, arg1, arg2 } => {
                func(arg1.eval(ctx), arg2.eval(ctx))
            }
            ExprNode::SubExpr { inner } => inner.eval(ctx),
        }
    }
}

/// Compiled expression that can be evaluated
pub struct Expression {
    root: ExprNode,
    context: ExpressionContext,
}

impl Expression {
    /// Evaluate the expression with current variable bindings
    pub fn eval(&self) -> f64 {
        self.root.eval(&self.context)
    }

    /// Bind a variable to a value pointer
    ///
    /// # Safety
    /// The pointer must remain valid for the lifetime of the expression or until unbound
    pub unsafe fn bind_ptr(&mut self, name: &str, ptr: *mut f64) {
        self.context.bind_ptr(name, ptr);
    }

    /// Bind a variable by reference (safe wrapper)
    pub fn bind(&mut self, name: &str, value: &mut f64) {
        unsafe {
            self.bind_ptr(name, value as *mut f64);
        }
    }

    /// Get the context
    pub fn context(&self) -> &ExpressionContext {
        &self.context
    }

    /// Get mutable context
    pub fn context_mut(&mut self) -> &mut ExpressionContext {
        &mut self.context
    }

    /// Cylindrical coordinate: x = r * cos(theta) where r = eval()
    pub fn cyl_x(&self, theta: f64) -> f64 {
        self.eval() * theta.cos()
    }

    /// Cylindrical coordinate: y = r * sin(theta) where r = eval()
    pub fn cyl_y(&self, theta: f64) -> f64 {
        self.eval() * theta.sin()
    }
}

/// Expression compiler
pub struct ExpressionCompiler {
    tokenizer: Tokenizer,
    functions: FunctionRegistry,
    constants: ConstantRegistry,
}

impl Default for ExpressionCompiler {
    fn default() -> Self {
        Self::new()
    }
}

impl ExpressionCompiler {
    /// Create a new expression compiler
    pub fn new() -> Self {
        Self {
            tokenizer: Tokenizer::new(),
            functions: FunctionRegistry::new(),
            constants: ConstantRegistry::new(),
        }
    }

    /// Compile an expression string
    pub fn compile(&self, source: &str) -> Result<Expression, String> {
        let tokens = self.tokenizer.tokenize(source);
        self.compile_tokens(&tokens)
    }

    /// Compile from tokens
    fn compile_tokens(&self, tokens: &[Token]) -> Result<Expression, String> {
        if tokens.is_empty() {
            return Err("Empty expression".to_string());
        }

        let mut pos = 0;
        let root = self.compile_additive(tokens, &mut pos)?;

        Ok(Expression {
            root,
            context: ExpressionContext::new(),
        })
    }

    fn skip_spaces<'a>(&self, tokens: &'a [Token], pos: &mut usize) -> Option<&'a Token> {
        while *pos < tokens.len() {
            if tokens[*pos].token_type != TokenType::Space {
                return Some(&tokens[*pos]);
            }
            *pos += 1;
        }
        None
    }

    fn compile_additive(&self, tokens: &[Token], pos: &mut usize) -> Result<ExprNode, String> {
        let mut left = self.compile_multiplicative(tokens, pos)?;

        loop {
            self.skip_spaces(tokens, pos);
            if *pos >= tokens.len() {
                break;
            }

            let token = &tokens[*pos];
            if token.token_type != TokenType::BinaryOperator {
                break;
            }

            let op = match token.value.as_str() {
                "+" => BinaryOperator::Plus,
                "-" => BinaryOperator::Minus,
                _ => break,
            };

            *pos += 1;
            self.skip_spaces(tokens, pos);

            let right = self.compile_multiplicative(tokens, pos)?;
            left = ExprNode::Binary {
                op,
                left: Box::new(left),
                right: Box::new(right),
            };
        }

        Ok(left)
    }

    fn compile_multiplicative(&self, tokens: &[Token], pos: &mut usize) -> Result<ExprNode, String> {
        let mut left = self.compile_unary(tokens, pos)?;

        loop {
            self.skip_spaces(tokens, pos);
            if *pos >= tokens.len() {
                break;
            }

            let token = &tokens[*pos];
            if token.token_type != TokenType::BinaryOperator {
                break;
            }

            let op = match token.value.as_str() {
                "*" => BinaryOperator::Multiply,
                "/" => BinaryOperator::Divide,
                _ => break,
            };

            *pos += 1;
            self.skip_spaces(tokens, pos);

            let right = self.compile_unary(tokens, pos)?;
            left = ExprNode::Binary {
                op,
                left: Box::new(left),
                right: Box::new(right),
            };
        }

        Ok(left)
    }

    fn compile_unary(&self, tokens: &[Token], pos: &mut usize) -> Result<ExprNode, String> {
        self.skip_spaces(tokens, pos);
        if *pos >= tokens.len() {
            return Err("Unexpected end of expression".to_string());
        }

        let token = &tokens[*pos];
        if token.token_type == TokenType::UnaryOperator {
            let op = match token.value.as_str() {
                "+" => UnaryOperator::Plus,
                "-" => UnaryOperator::Minus,
                _ => return Err(format!("Unknown unary operator: {}", token.value)),
            };

            *pos += 1;
            self.skip_spaces(tokens, pos);

            let operand = self.compile_power(tokens, pos)?;

            if matches!(op, UnaryOperator::Plus) {
                return Ok(operand);
            }

            return Ok(ExprNode::Unary {
                op,
                operand: Box::new(operand),
            });
        }

        self.compile_power(tokens, pos)
    }

    fn compile_power(&self, tokens: &[Token], pos: &mut usize) -> Result<ExprNode, String> {
        let mut left = self.compile_primary(tokens, pos)?;

        loop {
            self.skip_spaces(tokens, pos);
            if *pos >= tokens.len() {
                break;
            }

            let token = &tokens[*pos];
            if token.token_type != TokenType::BinaryOperator || token.value != "**" {
                break;
            }

            *pos += 1;
            self.skip_spaces(tokens, pos);

            // Right-associative: parse right side as power
            let right = self.compile_power(tokens, pos)?;
            left = ExprNode::Binary {
                op: BinaryOperator::Power,
                left: Box::new(left),
                right: Box::new(right),
            };
        }

        Ok(left)
    }

    fn compile_primary(&self, tokens: &[Token], pos: &mut usize) -> Result<ExprNode, String> {
        self.skip_spaces(tokens, pos);
        if *pos >= tokens.len() {
            return Err("Unexpected end of expression".to_string());
        }

        let token = &tokens[*pos];

        // Number
        if token.token_type == TokenType::Number {
            let value: f64 = token.value.parse().map_err(|e| format!("Invalid number: {}", e))?;
            *pos += 1;
            return Ok(ExprNode::Constant(value));
        }

        // Parenthesized expression
        if token.token_type == TokenType::ExpressionBound && token.value == "(" {
            *pos += 1;
            self.skip_spaces(tokens, pos);

            let inner = self.compile_additive(tokens, pos)?;

            self.skip_spaces(tokens, pos);
            if *pos >= tokens.len() {
                return Err("Missing closing parenthesis".to_string());
            }

            let close = &tokens[*pos];
            if close.token_type != TokenType::ExpressionBound || close.value != ")" {
                return Err("Expected closing parenthesis".to_string());
            }
            *pos += 1;

            return Ok(ExprNode::SubExpr {
                inner: Box::new(inner),
            });
        }

        // Identifier (function, constant, or variable)
        if token.token_type == TokenType::Identifier {
            let name = token.value.clone();
            *pos += 1;

            self.skip_spaces(tokens, pos);

            // Check if it's a function call
            if *pos < tokens.len()
                && tokens[*pos].token_type == TokenType::ExpressionBound
                && tokens[*pos].value == "("
            {
                *pos += 1; // consume '('

                // Parse arguments
                let mut args = Vec::new();

                self.skip_spaces(tokens, pos);

                // Empty arg list?
                if *pos < tokens.len()
                    && tokens[*pos].token_type == TokenType::ExpressionBound
                    && tokens[*pos].value == ")"
                {
                    *pos += 1;
                } else {
                    // Parse first argument
                    args.push(self.compile_additive(tokens, pos)?);

                    // More arguments?
                    loop {
                        self.skip_spaces(tokens, pos);
                        if *pos >= tokens.len() {
                            break;
                        }

                        if tokens[*pos].token_type == TokenType::Comma {
                            *pos += 1;
                            self.skip_spaces(tokens, pos);
                            args.push(self.compile_additive(tokens, pos)?);
                        } else if tokens[*pos].token_type == TokenType::ExpressionBound
                            && tokens[*pos].value == ")"
                        {
                            *pos += 1;
                            break;
                        } else {
                            return Err("Expected ',' or ')' in function call".to_string());
                        }
                    }
                }

                // Look up function
                if let Some(entry) = self.functions.get(&name) {
                    if entry.arity == 1 {
                        if args.len() != 1 {
                            return Err(format!(
                                "Function {} expects 1 argument, got {}",
                                name,
                                args.len()
                            ));
                        }
                        return Ok(ExprNode::UnaryFunc {
                            func: entry.unary_fn.unwrap(),
                            arg: Box::new(args.remove(0)),
                        });
                    } else if entry.arity == 2 {
                        if args.len() != 2 {
                            return Err(format!(
                                "Function {} expects 2 arguments, got {}",
                                name,
                                args.len()
                            ));
                        }
                        let arg2 = args.remove(1);
                        let arg1 = args.remove(0);
                        return Ok(ExprNode::BinaryFunc {
                            func: entry.binary_fn.unwrap(),
                            arg1: Box::new(arg1),
                            arg2: Box::new(arg2),
                        });
                    }
                }

                return Err(format!("Unknown function: {}", name));
            }

            // Check if it's a constant
            if let Some(value) = self.constants.get(&name) {
                return Ok(ExprNode::Constant(value));
            }

            // It's a variable
            return Ok(ExprNode::Variable(name));
        }

        Err(format!("Unexpected token: {:?}", token.value))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_compile_constant() {
        let compiler = ExpressionCompiler::new();
        let expr = compiler.compile("42").unwrap();
        assert!((expr.eval() - 42.0).abs() < 1e-10);
    }

    #[test]
    fn test_compile_float() {
        let compiler = ExpressionCompiler::new();
        let expr = compiler.compile("3.14159").unwrap();
        assert!((expr.eval() - 3.14159).abs() < 1e-10);
    }

    #[test]
    fn test_compile_addition() {
        let compiler = ExpressionCompiler::new();
        let expr = compiler.compile("1 + 2").unwrap();
        assert!((expr.eval() - 3.0).abs() < 1e-10);
    }

    #[test]
    fn test_compile_precedence() {
        let compiler = ExpressionCompiler::new();
        let expr = compiler.compile("1 + 2 * 3").unwrap();
        assert!((expr.eval() - 7.0).abs() < 1e-10);
    }

    #[test]
    fn test_compile_parentheses() {
        let compiler = ExpressionCompiler::new();
        let expr = compiler.compile("(1 + 2) * 3").unwrap();
        assert!((expr.eval() - 9.0).abs() < 1e-10);
    }

    #[test]
    fn test_compile_power() {
        let compiler = ExpressionCompiler::new();
        let expr = compiler.compile("2 ** 3").unwrap();
        assert!((expr.eval() - 8.0).abs() < 1e-10);
    }

    #[test]
    fn test_compile_unary_minus() {
        let compiler = ExpressionCompiler::new();
        let expr = compiler.compile("-5").unwrap();
        assert!((expr.eval() - (-5.0)).abs() < 1e-10);
    }

    #[test]
    fn test_compile_function() {
        let compiler = ExpressionCompiler::new();
        let expr = compiler.compile("sin(0)").unwrap();
        assert!(expr.eval().abs() < 1e-10);
    }

    #[test]
    fn test_compile_pi() {
        let compiler = ExpressionCompiler::new();
        let expr = compiler.compile("PI").unwrap();
        assert!((expr.eval() - std::f64::consts::PI).abs() < 1e-10);
    }

    #[test]
    fn test_compile_variable() {
        let compiler = ExpressionCompiler::new();
        let mut expr = compiler.compile("x + 1").unwrap();

        let mut x = 5.0;
        expr.bind("x", &mut x);

        assert!((expr.eval() - 6.0).abs() < 1e-10);

        // Value is read through the bound pointer in expr.eval()
        #[allow(unused_assignments)]
        { x = 10.0; }
        assert!((expr.eval() - 11.0).abs() < 1e-10);
    }

    #[test]
    fn test_compile_complex() {
        let compiler = ExpressionCompiler::new();
        let mut expr = compiler.compile("sin(theta) * r").unwrap();

        let mut theta = std::f64::consts::PI / 2.0;
        let mut r = 2.0;
        expr.bind("theta", &mut theta);
        expr.bind("r", &mut r);

        assert!((expr.eval() - 2.0).abs() < 1e-10);
    }

    #[test]
    fn test_cylindrical() {
        let compiler = ExpressionCompiler::new();
        let expr = compiler.compile("1").unwrap();

        let theta = std::f64::consts::PI / 2.0;
        assert!(expr.cyl_x(theta).abs() < 1e-10);
        assert!((expr.cyl_y(theta) - 1.0).abs() < 1e-10);
    }
}
