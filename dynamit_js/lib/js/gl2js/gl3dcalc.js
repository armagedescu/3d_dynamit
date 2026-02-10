"use strict";

//========================================
// 3DCalculator - Expression evaluator with symbolic differentiation
// JavaScript port of C++ expression_compiler.h
//========================================

//========================================
// Binary operators
//========================================
const BinaryOperator = {
   PLUS: 0,
   MINUS: 1,
   MULTIPLY: 2,
   DIVIDE: 3,
   POWER: 4,
   UNKNOWN: 5
};

//========================================
// Built-in constants
//========================================
const MathConstants = {
   PI: Math.PI,
   E: Math.E,
   TAU: Math.PI * 2,
   M_PI: Math.PI,
   M_E: Math.E,
   M_PI_2: Math.PI / 2,
   M_PI_4: Math.PI / 4,
   M_SQRT2: Math.SQRT2,
   M_SQRT1_2: Math.SQRT1_2,
   M_LN2: Math.LN2,
   M_LN10: Math.LN10,
   M_LOG2E: Math.LOG2E,
   M_LOG10E: Math.LOG10E,
   M_1_PI: 1 / Math.PI,
   M_2_PI: 2 / Math.PI,
   M_2_SQRTPI: 2 / Math.sqrt(Math.PI)
};

//========================================
// Function registry with derivatives
//========================================
const FunctionRegistry = {
   sin: { arity: 1, fn: Math.sin, deriv: Math.cos },
   cos: { arity: 1, fn: Math.cos, deriv: x => -Math.sin(x) },
   tan: { arity: 1, fn: Math.tan, deriv: x => { let c = Math.cos(x); return 1 / (c * c); } },
   sqrt: { arity: 1, fn: Math.sqrt, deriv: x => 0.5 / Math.sqrt(x) },
   exp: { arity: 1, fn: Math.exp, deriv: Math.exp },
   log: { arity: 1, fn: Math.log, deriv: x => 1 / x },
   abs: { arity: 1, fn: Math.abs, deriv: x => x >= 0 ? 1 : -1 },
   asin: { arity: 1, fn: Math.asin, deriv: x => 1 / Math.sqrt(1 - x * x) },
   acos: { arity: 1, fn: Math.acos, deriv: x => -1 / Math.sqrt(1 - x * x) },
   atan: { arity: 1, fn: Math.atan, deriv: x => 1 / (1 + x * x) },
   sinh: { arity: 1, fn: Math.sinh, deriv: Math.cosh },
   cosh: { arity: 1, fn: Math.cosh, deriv: Math.sinh },
   tanh: { arity: 1, fn: Math.tanh, deriv: x => { let t = Math.tanh(x); return 1 - t * t; } },
   floor: { arity: 1, fn: Math.floor, deriv: null },
   ceil: { arity: 1, fn: Math.ceil, deriv: null },
   round: { arity: 1, fn: Math.round, deriv: null },
   sign: { arity: 1, fn: Math.sign, deriv: null },
   pow: { arity: 2, fn: Math.pow, derivArg1: (a, b) => b * Math.pow(a, b - 1), derivArg2: (a, b) => Math.pow(a, b) * Math.log(a) },
   atan2: { arity: 2, fn: Math.atan2, derivArg1: (y, x) => x / (x * x + y * y), derivArg2: (y, x) => -y / (x * x + y * y) },
   min: { arity: 2, fn: Math.min, derivArg1: null, derivArg2: null },
   max: { arity: 2, fn: Math.max, derivArg1: null, derivArg2: null }
};

//========================================
// Expression base class
//========================================
class Expression {
   #context = new Map();
   
   bind(name, valueOrGetter) {
      this.#context.set(name.toLowerCase(), valueOrGetter);
      return this;
   }
   
   unbind(name) {
      this.#context.delete(name.toLowerCase());
      return this;
   }
   
   getBinding(name) {
      return this.#context.get(name.toLowerCase());
   }
   
   hasBinding(name) {
      return this.#context.has(name.toLowerCase());
   }
   
   get context() { return this.#context; }
   
   eval() { throw new Error("eval() must be implemented"); }
   derivative(wrt) { throw new Error("derivative() must be implemented"); }
   clone() { throw new Error("clone() must be implemented"); }
   isConstant() { return false; }
   
   cylX(theta) { return this.eval() * Math.cos(theta); }
   cylY(theta) { return this.eval() * Math.sin(theta); }
}

//========================================
// Number constant expression
//========================================
class NumberConstantExpression extends Expression {
   #value;
   
   constructor(value) {
      super();
      this.#value = value;
   }
   
   eval() { return this.#value; }
   isConstant() { return true; }
   
   derivative(wrt) {
      return new NumberConstantExpression(0);
   }
   
   clone() {
      return new NumberConstantExpression(this.#value);
   }
}

//========================================
// Variable expression
//========================================
class VariableExpression extends Expression {
   #name;
   #root = null;
   
   constructor(name) {
      super();
      this.#name = name.toLowerCase();
   }
   
   get name() { return this.#name; }
   
   setRoot(root) { this.#root = root; }
   
   eval() {
      let binding = this.#root ? this.#root.getBinding(this.#name) : this.getBinding(this.#name);
      if (binding === undefined) {
         throw new Error(`Unbound variable: ${this.#name}`);
      }
      return typeof binding === 'function' ? binding() : binding;
   }
   
   isConstant() { return false; }
   
   derivative(wrt) {
      return new NumberConstantExpression(this.#name === wrt.toLowerCase() ? 1 : 0);
   }
   
   clone() {
      let c = new VariableExpression(this.#name);
      c.#root = this.#root;
      return c;
   }
}

//========================================
// Unary expression (prefix +/-)
//========================================
class UnaryExpression extends Expression {
   #operand;
   #op; // 'plus' or 'minus'
   
   constructor(op, operand) {
      super();
      this.#op = op;
      this.#operand = operand;
   }
   
   eval() {
      let val = this.#operand.eval();
      return this.#op === 'minus' ? -val : val;
   }
   
   isConstant() { return this.#operand.isConstant(); }
   
   derivative(wrt) {
      let du = this.#operand.derivative(wrt);
      if (this.#op === 'minus') {
         return new UnaryExpression('minus', du);
      }
      return du;
   }
   
   clone() {
      return new UnaryExpression(this.#op, this.#operand.clone());
   }
}

//========================================
// Binary expression (+, -, *, /, **)
//========================================
class BinaryExpression extends Expression {
   #left;
   #right;
   #op;
   
   constructor(op, left, right) {
      super();
      this.#op = op;
      this.#left = left;
      this.#right = right;
   }
   
   eval() {
      let l = this.#left.eval();
      let r = this.#right.eval();
      switch (this.#op) {
         case BinaryOperator.PLUS: return l + r;
         case BinaryOperator.MINUS: return l - r;
         case BinaryOperator.MULTIPLY: return l * r;
         case BinaryOperator.DIVIDE: return l / r;
         case BinaryOperator.POWER: return Math.pow(l, r);
         default: throw new Error("Unknown operator");
      }
   }
   
   isConstant() { return this.#left.isConstant() && this.#right.isConstant(); }
   
   derivative(wrt) {
      let dl = this.#left.derivative(wrt);
      let dr = this.#right.derivative(wrt);
      
      switch (this.#op) {
         case BinaryOperator.PLUS:
            return new BinaryExpression(BinaryOperator.PLUS, dl, dr);
         case BinaryOperator.MINUS:
            return new BinaryExpression(BinaryOperator.MINUS, dl, dr);
         case BinaryOperator.MULTIPLY:
            // Product rule: d(a*b) = a*db + da*b
            return new BinaryExpression(BinaryOperator.PLUS,
               new BinaryExpression(BinaryOperator.MULTIPLY, this.#left.clone(), dr),
               new BinaryExpression(BinaryOperator.MULTIPLY, dl, this.#right.clone()));
         case BinaryOperator.DIVIDE:
            // Quotient rule: d(a/b) = (da*b - a*db) / b²
            return new BinaryExpression(BinaryOperator.DIVIDE,
               new BinaryExpression(BinaryOperator.MINUS,
                  new BinaryExpression(BinaryOperator.MULTIPLY, dl, this.#right.clone()),
                  new BinaryExpression(BinaryOperator.MULTIPLY, this.#left.clone(), dr)),
               new BinaryExpression(BinaryOperator.MULTIPLY, this.#right.clone(), this.#right.clone()));
         case BinaryOperator.POWER:
            if (this.#right.isConstant()) {
               // d(a^n) = n * a^(n-1) * da
               let n = this.#right.eval();
               return new BinaryExpression(BinaryOperator.MULTIPLY,
                  new BinaryExpression(BinaryOperator.MULTIPLY,
                     new NumberConstantExpression(n),
                     new BinaryExpression(BinaryOperator.POWER, this.#left.clone(), new NumberConstantExpression(n - 1))),
                  dl);
            } else {
               throw new Error("Derivative of variable exponent not implemented");
            }
         default:
            throw new Error("Unknown operator for derivative");
      }
   }
   
   clone() {
      return new BinaryExpression(this.#op, this.#left.clone(), this.#right.clone());
   }
}

//========================================
// Unary function expression
//========================================
class UnaryFunctionExpression extends Expression {
   #arg;
   #fn;
   #derivFn;
   
   constructor(fn, derivFn, arg) {
      super();
      this.#fn = fn;
      this.#derivFn = derivFn;
      this.#arg = arg;
   }
   
   eval() { return this.#fn(this.#arg.eval()); }
   isConstant() { return this.#arg.isConstant(); }
   
   derivative(wrt) {
      if (!this.#derivFn) throw new Error("No derivative for this function");
      // Chain rule: d(f(u)) = f'(u) * du
      let du = this.#arg.derivative(wrt);
      return new BinaryExpression(BinaryOperator.MULTIPLY,
         new UnaryFunctionExpression(this.#derivFn, null, this.#arg.clone()),
         du);
   }
   
   clone() {
      return new UnaryFunctionExpression(this.#fn, this.#derivFn, this.#arg.clone());
   }
}

//========================================
// Binary function expression
//========================================
class BinaryFunctionExpression extends Expression {
   #arg1;
   #arg2;
   #fn;
   #derivArg1;
   #derivArg2;
   
   constructor(fn, derivArg1, derivArg2, arg1, arg2) {
      super();
      this.#fn = fn;
      this.#derivArg1 = derivArg1;
      this.#derivArg2 = derivArg2;
      this.#arg1 = arg1;
      this.#arg2 = arg2;
   }
   
   eval() { return this.#fn(this.#arg1.eval(), this.#arg2.eval()); }
   isConstant() { return this.#arg1.isConstant() && this.#arg2.isConstant(); }
   
   derivative(wrt) {
      if (!this.#derivArg1 || !this.#derivArg2) throw new Error("No partial derivatives for this function");
      let du = this.#arg1.derivative(wrt);
      let dv = this.#arg2.derivative(wrt);
      // d(f(u,v)) = ∂f/∂u * du + ∂f/∂v * dv
      return new BinaryExpression(BinaryOperator.PLUS,
         new BinaryExpression(BinaryOperator.MULTIPLY,
            new BinaryFunctionExpression(this.#derivArg1, null, null, this.#arg1.clone(), this.#arg2.clone()),
            du),
         new BinaryExpression(BinaryOperator.MULTIPLY,
            new BinaryFunctionExpression(this.#derivArg2, null, null, this.#arg1.clone(), this.#arg2.clone()),
            dv));
   }
   
   clone() {
      return new BinaryFunctionExpression(this.#fn, this.#derivArg1, this.#derivArg2, this.#arg1.clone(), this.#arg2.clone());
   }
}

//========================================
// Sub-expression (parentheses)
//========================================
class SubExpression extends Expression {
   #inner;
   
   constructor(inner) {
      super();
      this.#inner = inner;
   }
   
   eval() { return this.#inner.eval(); }
   isConstant() { return this.#inner.isConstant(); }
   derivative(wrt) { return this.#inner.derivative(wrt); }
   clone() { return new SubExpression(this.#inner.clone()); }
}

//========================================
// Simplify expression (constant folding)
//========================================
function simplify(expr) {
   if (expr.isConstant()) {
      return new NumberConstantExpression(expr.eval());
   }
   return expr;
}

//========================================
// Expression Compiler - recursive descent parser
//========================================
class ExpressionCompiler {
   #variables = [];
   #pos = 0;
   #formula = "";
   
   compile(formula) {
      this.#formula = formula;
      this.#pos = 0;
      this.#variables = [];
      
      let expr = this.#parseAdditive();
      
      // Link all variables to root
      for (let v of this.#variables) {
         v.setRoot(expr);
      }
      
      return expr;
   }
   
   #peek() { return this.#formula[this.#pos] || ''; }
   #advance() { return this.#formula[this.#pos++] || ''; }
   #skipSpaces() { while (/\s/.test(this.#peek())) this.#advance(); }
   
   #parseAdditive() {
      let left = this.#parseMultiplicative();
      
      while (true) {
         this.#skipSpaces();
         let c = this.#peek();
         if (c === '+' || c === '-') {
            this.#advance();
            let right = this.#parseMultiplicative();
            left = new BinaryExpression(c === '+' ? BinaryOperator.PLUS : BinaryOperator.MINUS, left, right);
         } else {
            break;
         }
      }
      return left;
   }
   
   #parseMultiplicative() {
      let left = this.#parseUnary();
      
      while (true) {
         this.#skipSpaces();
         let c = this.#peek();
         if (c === '*' && this.#formula[this.#pos + 1] !== '*') {
            this.#advance();
            let right = this.#parseUnary();
            left = new BinaryExpression(BinaryOperator.MULTIPLY, left, right);
         } else if (c === '/') {
            this.#advance();
            let right = this.#parseUnary();
            left = new BinaryExpression(BinaryOperator.DIVIDE, left, right);
         } else {
            break;
         }
      }
      return left;
   }
   
   #parseUnary() {
      this.#skipSpaces();
      let c = this.#peek();
      if (c === '+') {
         this.#advance();
         return this.#parsePower();
      } else if (c === '-') {
         this.#advance();
         return new UnaryExpression('minus', this.#parsePower());
      }
      return this.#parsePower();
   }
   
   #parsePower() {
      let left = this.#parsePrimary();
      
      this.#skipSpaces();
      if (this.#formula.substr(this.#pos, 2) === '**') {
         this.#pos += 2;
         let right = this.#parsePower(); // right-associative
         return new BinaryExpression(BinaryOperator.POWER, left, right);
      }
      return left;
   }
   
   #parsePrimary() {
      this.#skipSpaces();
      let c = this.#peek();
      
      // Parentheses
      if (c === '(') {
         this.#advance();
         let inner = this.#parseAdditive();
         this.#skipSpaces();
         if (this.#peek() === ')') this.#advance();
         return new SubExpression(inner);
      }
      
      // Number
      if (/[0-9.]/.test(c)) {
         return this.#parseNumber();
      }
      
      // Identifier (function, constant, or variable)
      if (/[a-zA-Z_]/.test(c)) {
         return this.#parseIdentifier();
      }
      
      throw new Error(`Unexpected character: ${c} at position ${this.#pos}`);
   }
   
   #parseNumber() {
      let start = this.#pos;
      while (/[0-9.eE+-]/.test(this.#peek())) {
         if ((this.#peek() === '+' || this.#peek() === '-') && 
             this.#pos > start && 
             !/[eE]/.test(this.#formula[this.#pos - 1])) {
            break;
         }
         this.#advance();
      }
      return new NumberConstantExpression(parseFloat(this.#formula.substring(start, this.#pos)));
   }
   
   #parseIdentifier() {
      let start = this.#pos;
      while (/[a-zA-Z0-9_]/.test(this.#peek())) this.#advance();
      let name = this.#formula.substring(start, this.#pos);
      let nameLower = name.toLowerCase();
      
      this.#skipSpaces();
      
      // Check if function call
      if (this.#peek() === '(') {
         this.#advance();
         let args = this.#parseArgList();
         this.#skipSpaces();
         if (this.#peek() === ')') this.#advance();
         
         let fnEntry = FunctionRegistry[nameLower];
         if (!fnEntry) throw new Error(`Unknown function: ${name}`);
         
         if (fnEntry.arity === 1) {
            if (args.length !== 1) throw new Error(`${name} expects 1 argument`);
            return new UnaryFunctionExpression(fnEntry.fn, fnEntry.deriv, args[0]);
         } else if (fnEntry.arity === 2) {
            if (args.length !== 2) throw new Error(`${name} expects 2 arguments`);
            return new BinaryFunctionExpression(fnEntry.fn, fnEntry.derivArg1, fnEntry.derivArg2, args[0], args[1]);
         }
      }
      
      // Check if constant
      if (MathConstants.hasOwnProperty(nameLower) || MathConstants.hasOwnProperty(name)) {
         return new NumberConstantExpression(MathConstants[nameLower] || MathConstants[name]);
      }
      
      // Variable
      let v = new VariableExpression(name);
      this.#variables.push(v);
      return v;
   }
   
   #parseArgList() {
      let args = [];
      this.#skipSpaces();
      if (this.#peek() === ')') return args;
      
      args.push(this.#parseAdditive());
      
      while (true) {
         this.#skipSpaces();
         if (this.#peek() === ',') {
            this.#advance();
            args.push(this.#parseAdditive());
         } else {
            break;
         }
      }
      return args;
   }
}