//! # Calculator 3D
//!
//! Mathematical expression compiler for 3D graphics.
//! Rust port of the C++ 3DCalculator library.
//!
//! ## Features
//! - Tokenize mathematical expressions
//! - Compile to evaluable AST
//! - Variable binding with context
//! - Built-in math functions (sin, cos, sqrt, etc.)
//! - Built-in constants (PI, E, TAU, etc.)
//! - Symbolic differentiation
//!
//! ## Example
//! ```
//! use calculator_3d::{ExpressionCompiler, ExpressionContext};
//!
//! let compiler = ExpressionCompiler::new();
//! let mut expr = compiler.compile("sin(theta) * r").unwrap();
//!
//! let mut theta = 0.0_f64;
//! let mut r = 1.0_f64;
//! expr.bind("theta", &mut theta);
//! expr.bind("r", &mut r);
//!
//! theta = std::f64::consts::PI / 2.0;
//! r = 2.0;
//! assert!((expr.eval() - 2.0).abs() < 1e-10);
//! ```

mod syntax_tree;
mod tokenizer;
mod compiler;

pub use syntax_tree::SyntaxTree;
pub use tokenizer::{Token, TokenType, Tokenizer};
pub use compiler::{
    Expression, ExpressionCompiler, ExpressionContext,
    BinaryOperator, UnaryOperator,
};
