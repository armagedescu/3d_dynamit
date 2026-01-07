#pragma once
#ifndef __EXPRESSION_COMPILER_H__
#define __EXPRESSION_COMPILER_H__

#include <memory>
#include <cmath>
#include <functional>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <locale>
#include <stdexcept>
#include "expression_tokenizer.h"

namespace expresie_tokenizer
{
//expression tokenizers

enum binary_operator : int
{
	plus,
	minus,
	multiply,
	divide,
	power,
	unknown
};

enum unary_operator : int
{
	unary_plus,
	unary_minus,
	unary_unknown
};

// Operator precedence levels (higher = binds tighter)
inline int get_precedence(binary_operator op)
{
	switch (op)
	{
	case plus:
	case minus:
		return 1;
	case multiply:
	case divide:
		return 2;
	case power:
		return 4;  // Highest for binary ops
	default:
		return 0;
	}
}

// Power is right-associative
inline bool is_right_associative(binary_operator op)
{
	return op == power;
}

//========================================
// Constant registry - built-in math constants
//========================================
class constant_registry
{
	std::unordered_map<std::wstring, long double> map_;
	
	static std::wstring to_lower(const std::wstring& name)
	{
		std::wstring lname = name;
		std::transform(lname.begin(), lname.end(), lname.begin(), towlower);
		return lname;
	}
	
	constant_registry()
	{
		// Math constants from <corecrt_math_defines.h>
		register_const(L"M_E",        2.718281828459045235360287471352662498L);
		register_const(L"M_LOG2E",    1.442695040888963407359924681001892137L);
		register_const(L"M_LOG10E",   0.434294481903251827651128918916605082L);
		register_const(L"M_LN2",      0.693147180559945309417232121458176568L);
		register_const(L"M_LN10",     2.302585092994045684017991454684364208L);
		register_const(L"M_PI",       3.141592653589793238462643383279502884L);
		register_const(L"M_PI_2",     1.570796326794896619231321691639751442L);
		register_const(L"M_PI_4",     0.785398163397448309615660845819875721L);
		register_const(L"M_1_PI",     0.318309886183790671537767526745028724L);
		register_const(L"M_2_PI",     0.636619772367581343075535053490057448L);
		register_const(L"M_2_SQRTPI", 1.128379167095512573896158903121545172L);
		register_const(L"M_SQRT2",    1.414213562373095048801688724209698079L);
		register_const(L"M_SQRT1_2",  0.707106781186547524400844362104849039L);
		
		// Convenient aliases
		register_const(L"PI",  3.141592653589793238462643383279502884L);
		register_const(L"E",   2.718281828459045235360287471352662498L);
		register_const(L"TAU", 6.283185307179586476925286766559005768L);
	}
	
public:
	static constant_registry& instance()
	{
		static constant_registry inst;
		return inst;
	}
	
	void register_const(const std::wstring& name, long double value)
	{
		map_[to_lower(name)] = value;
	}
	
	bool has(const std::wstring& name) const
	{
		return map_.find(to_lower(name)) != map_.end();
	}
	
	long double get(const std::wstring& name) const
	{
		std::unordered_map<std::wstring, long double>::const_iterator it = map_.find(to_lower(name));
		if (it == map_.end()) return 0.0L;
		return it->second;
	}
};

//========================================
// Expression Context - variable bindings owned by expression
//========================================
class expression_context
{
	std::unordered_map<std::wstring, long double*> bindings_;
	
	static std::wstring to_lower(const std::wstring& name)
	{
		std::wstring lname = name;
		std::transform(lname.begin(), lname.end(), lname.begin(), towlower);
		return lname;
	}
	
public:
	expression_context() = default;
	
	// Copyable and movable
	expression_context(const expression_context&) = default;
	expression_context& operator=(const expression_context&) = default;
	expression_context(expression_context&&) = default;
	expression_context& operator=(expression_context&&) = default;
	
	void bind(const std::wstring& name, long double* ptr)
	{
		bindings_[to_lower(name)] = ptr;
	}
	
	void unbind(const std::wstring& name)
	{
		bindings_.erase(to_lower(name));
	}
	
	void clear()
	{
		bindings_.clear();
	}
	
	long double* get(const std::wstring& name) const
	{
		std::unordered_map<std::wstring, long double*>::const_iterator it = bindings_.find(to_lower(name));
		return (it != bindings_.end()) ? it->second : nullptr;
	}
	
	bool has(const std::wstring& name) const
	{
		return bindings_.find(to_lower(name)) != bindings_.end();
	}
	
	size_t size() const
	{
		return bindings_.size();
	}
};


class binary_expression;
class expression
{
protected:
	expression_context context_;  // Each expression owns its context
	
public:
	// Context management
	expression_context& context() { return context_; }
	const expression_context& context() const { return context_; }
	
	void bind(const std::wstring& name, long double* ptr) { context_.bind(name, ptr); }
	void unbind(const std::wstring& name) { context_.unbind(name); }
	
	virtual long double eval() = 0;
	
	// Cylindrical coordinate transformations
	// Expression evaluates to r, theta is passed as argument
	// cyl_x(theta) = r * cos(theta) where r = eval()
	long double cyl_x(long double theta)
	{
		return eval() * std::cosl(theta);
	}
	
	// cyl_y(theta) = r * sin(theta) where r = eval()
	long double cyl_y(long double theta)
	{
		return eval() * std::sinl(theta);
	}
	
	virtual bool is_unary() = 0;
	virtual bool is_binary() = 0;
	virtual bool is_subexpression() = 0;
	virtual bool is_constant() = 0;
	virtual expression*        get_left() = 0;
	virtual expression*        get_right() = 0;
	virtual binary_expression* get_binary() = 0;
	virtual void               set_left(expression* exp) = 0;
	virtual void               set_right(expression* exp) = 0;
	
	// Symbolic differentiation - returns new expression tree
	virtual std::unique_ptr<expression> derivative(const std::wstring& wrt) = 0;
	
	// Clone expression tree
	virtual std::unique_ptr<expression> clone() = 0;
	
	virtual ~expression() = default;
};

// Forward declarations for helper functions
std::unique_ptr<expression> make_constant(long double value);
std::unique_ptr<expression> make_binary(binary_operator op, std::unique_ptr<expression> left, std::unique_ptr<expression> right);
std::unique_ptr<expression> make_unary_func(long double (*func)(long double), std::unique_ptr<expression> arg);
std::unique_ptr<expression> make_binary_func(long double (*func)(long double, long double), std::unique_ptr<expression> arg1, std::unique_ptr<expression> arg2);

class constant_expression : public expression
{
public:
	virtual binary_expression* get_binary() { return nullptr; }
	virtual bool is_constant() { return true; }
	virtual bool is_subexpression() { return false; }
};

class sub_expression : public expression
{
public:
	std::unique_ptr<expression> inner_expression = nullptr;
	sub_expression() {}
	virtual long double eval() { return inner_expression->eval(); }
	virtual bool is_unary() { return true; }
	virtual bool is_binary() { return false; }
	virtual bool is_subexpression() { return true; }
	virtual bool is_constant() { return inner_expression->is_constant(); }

	virtual expression*        get_left() { return nullptr; }
	virtual expression*        get_right() { return nullptr; }
	virtual binary_expression* get_binary() { return nullptr; }
	virtual void               set_left(expression* exp) {}
	virtual void               set_right(expression* exp) {}
	
	virtual std::unique_ptr<expression> derivative(const std::wstring& wrt)
	{
		return inner_expression->derivative(wrt);
	}
	
	virtual std::unique_ptr<expression> clone()
	{
		std::unique_ptr<sub_expression> c = std::make_unique<sub_expression>();
		c->inner_expression = inner_expression->clone();
		return c;
	}
};

class number_constant_expression : public constant_expression
{
public:
	long double number = 0.0L;

	virtual long double eval() { return number; }
	virtual bool is_unary() { return true; }
	virtual bool is_binary() { return false; }
	virtual expression* get_left() { return nullptr; }
	virtual expression* get_right() { return nullptr; }
	virtual void set_left(expression* exp) {}
	virtual void set_right(expression* exp) {}
	
	// d/dx(c) = 0
	virtual std::unique_ptr<expression> derivative(const std::wstring& wrt)
	{
		return make_constant(0.0L);
	}
	
	virtual std::unique_ptr<expression> clone()
	{
		std::unique_ptr<number_constant_expression> c = std::make_unique<number_constant_expression>();
		c->number = number;
		return c;
	}
};

//========================================
// Variable Expression - looks up in root expression's context
//========================================
class variable_expression : public expression
{
public:
	std::wstring name;
	expression* root = nullptr;  // pointer to root expression that owns the context
	long double* cached_ptr = nullptr;  // cached for performance
	
	variable_expression(const std::wstring& var_name) : name(var_name) {}
	
	void set_root(expression* r) 
	{ 
		root = r; 
		cached_ptr = nullptr;  // invalidate cache
	}
	
	long double eval() override
	{
		// Try cached pointer first
		if (cached_ptr)
			return *cached_ptr;
		
		// Look up in root's context
		if (root)
		{
			cached_ptr = root->context().get(name);
			if (cached_ptr)
				return *cached_ptr;
		}
		
		throw std::runtime_error("Unbound variable: " + std::string(name.begin(), name.end()));
	}
	
	// Invalidate cache (call after rebinding)
	void invalidate_cache() { cached_ptr = nullptr; }
	
	virtual bool is_unary() { return true; }
	virtual bool is_binary() { return false; }
	virtual bool is_subexpression() { return false; }
	virtual bool is_constant() { return false; }
	virtual expression* get_left() { return nullptr; }
	virtual expression* get_right() { return nullptr; }
	virtual binary_expression* get_binary() { return nullptr; }
	virtual void set_left(expression* exp) {}
	virtual void set_right(expression* exp) {}
	
	// d/dx(x) = 1, d/dx(y) = 0
	virtual std::unique_ptr<expression> derivative(const std::wstring& wrt)
	{
		std::wstring lname = name;
		std::transform(lname.begin(), lname.end(), lname.begin(), towlower);
		std::wstring lwrt = wrt;
		std::transform(lwrt.begin(), lwrt.end(), lwrt.begin(), towlower);
		
		if (lname == lwrt)
			return make_constant(1.0L);
		else
			return make_constant(0.0L);
	}
	
	virtual std::unique_ptr<expression> clone()
	{
		std::unique_ptr<variable_expression> c = std::make_unique<variable_expression>(name);
		c->root = root;
		c->cached_ptr = nullptr;
		return c;
	}
};

//========================================
// Unary Expression (prefix operators: +, -)
//========================================
class unary_expression : public expression
{
public:
	std::unique_ptr<expression> operand = nullptr;
	unary_operator op = unary_unknown;

	// Function pointer for evaluation
	long double (*eval_func)(long double) = nullptr;

	long double eval() override
	{
		return eval_func(operand->eval());
	}

	virtual bool is_unary() { return true; }
	virtual bool is_binary() { return false; }
	virtual bool is_constant() { return operand->is_constant(); }
	virtual bool is_subexpression() { return false; }

	virtual expression* get_left() { return nullptr; }
	virtual expression* get_right() { return nullptr; }
	virtual binary_expression* get_binary() { return nullptr; }
	virtual void set_left(expression* exp) {}
	virtual void set_right(expression* exp) {}
	
	// d/dx(-u) = -du/dx
	virtual std::unique_ptr<expression> derivative(const std::wstring& wrt)
	{
		std::unique_ptr<expression> du = operand->derivative(wrt);
		
		if (op == unary_minus)
		{
			std::unique_ptr<unary_expression> result = std::make_unique<unary_expression>();
			result->op = unary_minus;
			result->eval_func = [](long double a) { return -a; };
			result->operand = std::move(du);
			return result;
		}
		// unary_plus: d/dx(+u) = du/dx
		return du;
	}
	
	virtual std::unique_ptr<expression> clone()
	{
		std::unique_ptr<unary_expression> c = std::make_unique<unary_expression>();
		c->op = op;
		c->eval_func = eval_func;
		c->operand = operand->clone();
		return c;
	}
};

class binary_expression : public expression
{
public:
	std::unique_ptr<expression> left = nullptr, right = nullptr;
	binary_operator op = expresie_tokenizer::unknown;

	// Function pointer for direct evaluation - no switch/case overhead
	long double (*eval_func_discrete)(long double, long double) = nullptr;

	long double eval()
	{
		return eval_func_discrete(left->eval(), right->eval());
	}

	virtual bool is_unary()         { return false; }
	virtual bool is_binary()        { return true; }
	virtual bool is_constant()      { return left->is_constant() && right->is_constant(); }
	virtual bool is_subexpression() { return false; }

	virtual expression* get_left() { return left.release(); }
	virtual expression* get_right() { return right.release(); }
	virtual binary_expression* get_binary() { return this; }
	virtual void set_left(expression* exp) { left.reset(exp); }
	virtual void set_right(expression* exp) { return right.reset(exp); }
	
	virtual std::unique_ptr<expression> derivative(const std::wstring& wrt)
	{
		std::unique_ptr<expression> dl = left->derivative(wrt);
		std::unique_ptr<expression> dr = right->derivative(wrt);
		
		switch (op)
		{
		case plus:
			// d/dx(a + b) = da + db
			return make_binary(plus, std::move(dl), std::move(dr));
			
		case minus:
			// d/dx(a - b) = da - db
			return make_binary(minus, std::move(dl), std::move(dr));
			
		case multiply:
			// d/dx(a * b) = a*db + da*b  (product rule)
			return make_binary(plus,
				make_binary(multiply, left->clone(), std::move(dr)),
				make_binary(multiply, std::move(dl), right->clone()));
			
		case divide:
			// d/dx(a / b) = (da*b - a*db) / b²  (quotient rule)
			return make_binary(divide,
				make_binary(minus,
					make_binary(multiply, std::move(dl), right->clone()),
					make_binary(multiply, left->clone(), std::move(dr))),
				make_binary(multiply, right->clone(), right->clone()));
			
		case power:
			// d/dx(a^b) where b is constant: b * a^(b-1) * da
			// For variable exponent: a^b * (db*ln(a) + b*da/a) - not implemented yet
			if (right->is_constant())
			{
				long double exp_val = right->eval();
				// b * a^(b-1) * da
				return make_binary(multiply,
					make_binary(multiply,
						make_constant(exp_val),
						make_binary(power, left->clone(), make_constant(exp_val - 1.0L))),
					std::move(dl));
			}
			else
			{
				throw std::runtime_error("Derivative of variable exponent not yet implemented");
			}
			
		default:
			throw std::runtime_error("Unknown binary operator for derivative");
		}
	}
	
	virtual std::unique_ptr<expression> clone()
	{
		std::unique_ptr<binary_expression> c = std::make_unique<binary_expression>();
		c->op = op;
		c->eval_func_discrete = eval_func_discrete;
		c->left = left->clone();
		c->right = right->clone();
		return c;
	}
};

//========================================
// Unary function expression - single argument, direct function pointer
//========================================
class unary_function_expression : public expression
{
public:
	std::unique_ptr<expression> arg;
	long double (*func)(long double) = nullptr;
	long double (*deriv_func)(long double) = nullptr;  // derivative of the function itself
	
	long double eval() override
	{
		return func(arg->eval());
	}
	
	virtual bool is_unary() { return true; }
	virtual bool is_binary() { return false; }
	virtual bool is_subexpression() { return false; }
	virtual bool is_constant() { return arg->is_constant(); }
	virtual expression* get_left() { return nullptr; }
	virtual expression* get_right() { return nullptr; }
	virtual binary_expression* get_binary() { return nullptr; }
	virtual void set_left(expression* exp) {}
	virtual void set_right(expression* exp) {}
	
	// d/dx(f(u)) = f'(u) * du/dx  (chain rule)
	virtual std::unique_ptr<expression> derivative(const std::wstring& wrt)
	{
		if (!deriv_func)
			throw std::runtime_error("No derivative defined for this function");
		
		std::unique_ptr<expression> du = arg->derivative(wrt);
		
		// f'(u) * du
		return make_binary(multiply,
			make_unary_func(deriv_func, arg->clone()),
			std::move(du));
	}
	
	virtual std::unique_ptr<expression> clone()
	{
		std::unique_ptr<unary_function_expression> c = std::make_unique<unary_function_expression>();
		c->func = func;
		c->deriv_func = deriv_func;
		c->arg = arg->clone();
		return c;
	}
};

//========================================
// Binary function expression - two arguments, direct function pointer
//========================================
class binary_function_expression : public expression
{
public:
	std::unique_ptr<expression> arg1;
	std::unique_ptr<expression> arg2;
	long double (*func)(long double, long double) = nullptr;
	// Partial derivatives
	long double (*deriv_func_arg1)(long double, long double) = nullptr;  // ∂f/∂arg1
	long double (*deriv_func_arg2)(long double, long double) = nullptr;  // ∂f/∂arg2
	
	long double eval() override
	{
		return func(arg1->eval(), arg2->eval());
	}
	
	virtual bool is_unary() { return false; }
	virtual bool is_binary() { return true; }
	virtual bool is_subexpression() { return false; }
	virtual bool is_constant() { return arg1->is_constant() && arg2->is_constant(); }
	virtual expression* get_left() { return nullptr; }
	virtual expression* get_right() { return nullptr; }
	virtual binary_expression* get_binary() { return nullptr; }
	virtual void set_left(expression* exp) {}
	virtual void set_right(expression* exp) {}
	
	// d/dx(f(u,v)) = ∂f/∂u * du/dx + ∂f/∂v * dv/dx
	virtual std::unique_ptr<expression> derivative(const std::wstring& wrt)
	{
		if (!deriv_func_arg1 || !deriv_func_arg2)
			throw std::runtime_error("No partial derivatives defined for this function");
		
		std::unique_ptr<expression> du = arg1->derivative(wrt);
		std::unique_ptr<expression> dv = arg2->derivative(wrt);
		
		// ∂f/∂u * du + ∂f/∂v * dv
		return make_binary(plus,
			make_binary(multiply,
				make_binary_func(deriv_func_arg1, arg1->clone(), arg2->clone()),
				std::move(du)),
			make_binary(multiply,
				make_binary_func(deriv_func_arg2, arg1->clone(), arg2->clone()),
				std::move(dv)));
	}
	
	virtual std::unique_ptr<expression> clone()
	{
		std::unique_ptr<binary_function_expression> c = std::make_unique<binary_function_expression>();
		c->func = func;
		c->deriv_func_arg1 = deriv_func_arg1;
		c->deriv_func_arg2 = deriv_func_arg2;
		c->arg1 = arg1->clone();
		c->arg2 = arg2->clone();
		return c;
	}
};

//========================================
// Helper functions for building expressions
//========================================
inline std::unique_ptr<expression> make_constant(long double value)
{
	std::unique_ptr<number_constant_expression> c = std::make_unique<number_constant_expression>();
	c->number = value;
	return c;
}

inline std::unique_ptr<expression> make_binary(binary_operator op, std::unique_ptr<expression> left, std::unique_ptr<expression> right)
{
	std::unique_ptr<binary_expression> b = std::make_unique<binary_expression>();
	b->op = op;
	b->left = std::move(left);
	b->right = std::move(right);
	
	switch (op)
	{
	case plus:
		b->eval_func_discrete = [](long double a, long double b) { return a + b; };
		break;
	case minus:
		b->eval_func_discrete = [](long double a, long double b) { return a - b; };
		break;
	case multiply:
		b->eval_func_discrete = [](long double a, long double b) { return a * b; };
		break;
	case divide:
		b->eval_func_discrete = [](long double a, long double b) { return a / b; };
		break;
	case power:
		b->eval_func_discrete = [](long double a, long double b) { return std::powl(a, b); };
		break;
	default:
		break;
	}
	return b;
}

inline std::unique_ptr<expression> make_unary_func(long double (*func)(long double), std::unique_ptr<expression> arg)
{
	std::unique_ptr<unary_function_expression> f = std::make_unique<unary_function_expression>();
	f->func = func;
	f->arg = std::move(arg);
	// deriv_func not set - caller must set if needed
	return f;
}

inline std::unique_ptr<expression> make_binary_func(long double (*func)(long double, long double), std::unique_ptr<expression> arg1, std::unique_ptr<expression> arg2)
{
	std::unique_ptr<binary_function_expression> f = std::make_unique<binary_function_expression>();
	f->func = func;
	f->arg1 = std::move(arg1);
	f->arg2 = std::move(arg2);
	// deriv_funcs not set - caller must set if needed
	return f;
}

//========================================
// Function registry - stores both unary and binary function pointers
//========================================
class function_registry
{
public:
	using unary_fn_t = long double (*)(long double);
	using binary_fn_t = long double (*)(long double, long double);
	
	struct function_entry
	{
		int arity = 0;
		unary_fn_t unary_func = nullptr;
		unary_fn_t unary_deriv = nullptr;  // derivative of unary function
		binary_fn_t binary_func = nullptr;
		binary_fn_t binary_deriv_arg1 = nullptr;  // ∂f/∂arg1
		binary_fn_t binary_deriv_arg2 = nullptr;  // ∂f/∂arg2
	};
	
private:
	std::unordered_map<std::wstring, function_entry> map_;
	
	static std::wstring to_lower(const std::wstring& name)
	{
		std::wstring lname = name;
		std::transform(lname.begin(), lname.end(), lname.begin(), towlower);
		return lname;
	}
	
	function_registry()
	{
		// Register unary functions with their derivatives
		register_unary(L"sin", std::sinl, std::cosl);
		register_unary(L"cos", std::cosl, [](long double x) { return -std::sinl(x); });
		register_unary(L"tan", std::tanl, [](long double x) { long double c = std::cosl(x); return 1.0L / (c * c); });
		register_unary(L"sqrt", std::sqrtl, [](long double x) { return 0.5L / std::sqrtl(x); });
		register_unary(L"exp", std::expl, std::expl);
		register_unary(L"log", std::logl, [](long double x) { return 1.0L / x; });
		register_unary(L"abs", std::fabsl, [](long double x) { return x >= 0 ? 1.0L : -1.0L; });
		register_unary(L"asin", std::asinl, [](long double x) { return 1.0L / std::sqrtl(1.0L - x * x); });
		register_unary(L"acos", std::acosl, [](long double x) { return -1.0L / std::sqrtl(1.0L - x * x); });
		register_unary(L"atan", std::atanl, [](long double x) { return 1.0L / (1.0L + x * x); });
		register_unary(L"sinh", std::sinhl, std::coshl);
		register_unary(L"cosh", std::coshl, std::sinhl);
		register_unary(L"tanh", std::tanhl, [](long double x) { long double t = std::tanhl(x); return 1.0L - t * t; });
		register_unary(L"floor", std::floorl, nullptr);
		register_unary(L"ceil", std::ceill, nullptr);
		register_unary(L"round", std::roundl, nullptr);
		
		// Register binary functions with partial derivatives
		register_binary(L"pow", std::powl,
			[](long double a, long double b) { return b * std::powl(a, b - 1.0L); },
			[](long double a, long double b) { return std::powl(a, b) * std::logl(a); });
		register_binary(L"atan2", std::atan2l,
			[](long double y, long double x) { return x / (x * x + y * y); },
			[](long double y, long double x) { return -y / (x * x + y * y); });
		register_binary(L"fmod", std::fmodl, nullptr, nullptr);
		register_binary(L"min", [](long double a, long double b) { return a < b ? a : b; }, nullptr, nullptr);
		register_binary(L"max", [](long double a, long double b) { return a > b ? a : b; }, nullptr, nullptr);
	}
	
public:
	static function_registry& instance()
	{
		static function_registry inst;
		return inst;
	}
	
	void register_unary(const std::wstring& name, unary_fn_t f, unary_fn_t deriv)
	{
		function_entry entry;
		entry.arity = 1;
		entry.unary_func = f;
		entry.unary_deriv = deriv;
		map_[to_lower(name)] = entry;
	}
	
	void register_binary(const std::wstring& name, binary_fn_t f, binary_fn_t deriv_arg1, binary_fn_t deriv_arg2)
	{
		function_entry entry;
		entry.arity = 2;
		entry.binary_func = f;
		entry.binary_deriv_arg1 = deriv_arg1;
		entry.binary_deriv_arg2 = deriv_arg2;
		map_[to_lower(name)] = entry;
	}
	
	bool has(const std::wstring& name) const
	{
		return map_.find(to_lower(name)) != map_.end();
	}
	
	const function_entry* get(const std::wstring& name) const
	{
		std::unordered_map<std::wstring, function_entry>::const_iterator it = map_.find(to_lower(name));
		if (it == map_.end()) return nullptr;
		return &it->second;
	}
};

//========================================
// Constant folding - simplify expression if constant
//========================================
inline std::unique_ptr<expression> simplify(std::unique_ptr<expression> expr)
{
	if (expr->is_constant())
	{
		long double value = expr->eval();
		return make_constant(value);
	}
	return expr;
}

//========================================
// Compiler - recursive descent using token map
// Compiler only builds expression tree, does not deal with bindings
//========================================

// Type alias for readability
using token_map = std::map<size_t, std::unique_ptr<token>>;
using token_map_iterator = token_map::const_iterator;

class expression_token_compiler
{
private:
	std::vector<variable_expression*> variables_;  // track variables for root assignment
	
public:
	expression_token_compiler() = default;
	
	// compile entry - returns root expression with all variables linked to it
	expression_token_reader tokenizer;

	std::unique_ptr<expression> compile(const wchar_t* formula)
	{
		std::map<size_t, std::unique_ptr<token>> lang = tokenizer.tokenize_main(formula);
		return this->compile(lang);
	}
	std::unique_ptr<expression> compile(const std::wstring& formula)
	{
		std::map<size_t, std::unique_ptr<token>> lang = tokenizer.tokenize_main(formula);
		return this->compile(lang);
	}
	std::unique_ptr<expression> compile(const token_map& tokenz)
	{
		if (tokenz.empty()) return nullptr;
		variables_.clear();
		size_t advance = tokenz.begin()->first;
		std::unique_ptr<expression> ret = compile_additive(tokenz, advance, advance);
		
		// Link all variables to root expression
		if (ret)
		{
			for (variable_expression* var : variables_)
			{
				var->set_root(ret.get());
			}
		}
		
		return ret;
	}

private:
	// helper: get iterator for a position (exact or lower_bound)
	static token_map_iterator it_at(const token_map& tokenz, size_t pos)
	{
		token_map_iterator it = tokenz.find(pos);
		if (it != tokenz.end()) return it;
		it = tokenz.lower_bound(pos);
		return it;
	}
	// helper: next token position (or same if end)
	static size_t next_pos(const token_map& tokenz, size_t pos)
	{
		token_map_iterator it = it_at(tokenz, pos);
		if (it == tokenz.end()) return pos;
		token_map_iterator itn = std::next(it);
		if (itn == tokenz.end()) return it->first;
		return itn->first;
	}
	// helper skip spaces: returns iterator to first non-space token at or after pos
	static token_map_iterator skip_spaces_it(const token_map& tokenz, token_map_iterator it)
	{
		while (it != tokenz.end() && it->second->type() == token_type::space) ++it;
		return it;
	}

public:
	// parse number literal
	std::unique_ptr<number_constant_expression> compile_constant_number(const token_map& tokenz, size_t start_pos, size_t& advance)
	{
		token_map_iterator it = it_at(tokenz, start_pos);
		it = skip_spaces_it(tokenz, it);
		if (it == tokenz.end()) return nullptr;
		if (it->second->type() == token_type::number)
		{
			std::unique_ptr<number_constant_expression> num = std::make_unique<number_constant_expression>();
			num->number = std::wcstold(it->second->value.c_str(), nullptr);
			token_map_iterator itn = std::next(it);
			if (itn == tokenz.end()) advance = it->first;
			else advance = itn->first;
			return num;
		}
		return nullptr;
	}

	// parse parenthesized subexpression
	std::unique_ptr<expression> compile_subexpression(const token_map& tokenz, size_t start_pos, size_t& advance)
	{
		token_map_iterator it = it_at(tokenz, start_pos);
		it = skip_spaces_it(tokenz, it);
		if (it == tokenz.end()) return nullptr;
		if (it->second->type() != token_type::expression_bound) return nullptr;
		if (it->second->value != L"(") return nullptr;
		// move to token after '('
		token_map_iterator it_after = std::next(it);
		it_after = skip_spaces_it(tokenz, it_after);
		size_t inner_start = (it_after == tokenz.end()) ? it->first : it_after->first;
		size_t inner_advance = inner_start;
		std::unique_ptr<expression> inner_expr = compile_additive(tokenz, inner_start, inner_advance);
		if (!inner_expr) return nullptr;
		// now inner_advance should point to token right after expression; expect ')'
		token_map_iterator it_after_inner = it_at(tokenz, inner_advance);
		it_after_inner = skip_spaces_it(tokenz, it_after_inner);
		if (it_after_inner == tokenz.end()) return nullptr;
		if (it_after_inner->second->type() != token_type::expression_bound) return nullptr;
		if (it_after_inner->second->value != L")") return nullptr;
		// set advance to token after ')'
		token_map_iterator it_after_paren = std::next(it_after_inner);
		if (it_after_paren == tokenz.end()) advance = it_after_inner->first;
		else advance = it_after_paren->first;
		std::unique_ptr<sub_expression> node = std::make_unique<sub_expression>();
		node->inner_expression = std::move(inner_expr);
		return node;
	}

	// parse variable
	std::unique_ptr<variable_expression> compile_variable(const token_map& tokenz, size_t start_pos, size_t& advance)
	{
		token_map_iterator it = it_at(tokenz, start_pos);
		it = skip_spaces_it(tokenz, it);
		if (it == tokenz.end()) return nullptr;
		if (it->second->type() != token_type::identifier) return nullptr;
		
		std::wstring name = it->second->value;
		
		// Check it's not a known function (functions are handled separately with parentheses)
		// Variables are identifiers NOT followed by '('
		token_map_iterator it_next = std::next(it);
		it_next = skip_spaces_it(tokenz, it_next);
		if (it_next != tokenz.end() && 
		    it_next->second->type() == token_type::expression_bound && 
		    it_next->second->value == L"(")
		{
			// This is a function call, not a variable
			return nullptr;
		}
		
		std::unique_ptr<variable_expression> var = std::make_unique<variable_expression>(name);
		variables_.push_back(var.get());  // track for root assignment
		
		if (it_next == tokenz.end()) advance = it->first;
		else advance = it_next->first;
		return var;
	}

	// primary: number | (expr) | function-call | constant | variable
	std::unique_ptr<expression> compile_primary(const token_map& tokenz, size_t start_pos, size_t& advance)
	{
		token_map_iterator it = it_at(tokenz, start_pos);
		it = skip_spaces_it(tokenz, it);
		if (it == tokenz.end()) return nullptr;

		// identifier -- possible function call, constant, or variable
		if (it->second->type() == token_type::identifier)
		{
			std::wstring name = it->second->value;
			token_map_iterator it_next = std::next(it);
			it_next = skip_spaces_it(tokenz, it_next);
			if (it_next != tokenz.end() && it_next->second->type() == token_type::expression_bound && it_next->second->value == L"(")
			{
				// function call - resolve function at compile time
				const function_registry::function_entry* func_entry = function_registry::instance().get(name);
				if (!func_entry)
				{
					throw std::runtime_error("Unknown function: " + std::string(name.begin(), name.end()));
				}
				
				// Parse arguments
				std::vector<std::unique_ptr<expression>> args;
				
				// position after '('
				token_map_iterator it_after_lparen = std::next(it_next);
				it_after_lparen = skip_spaces_it(tokenz, it_after_lparen);
				
				// empty arg list?
				if (!(it_after_lparen != tokenz.end() && it_after_lparen->second->type() == token_type::expression_bound && it_after_lparen->second->value == L")"))
				{
					// parse comma-separated arguments
					token_map_iterator cur_it = it_after_lparen;
					while (cur_it != tokenz.end())
					{
						size_t arg_start = cur_it->first;
						size_t arg_end = arg_start;
						std::unique_ptr<expression> arg_expr = compile_additive(tokenz, arg_start, arg_end);
						if (!arg_expr) return nullptr;
						args.push_back(std::move(arg_expr));
						// move to token at arg_end
						cur_it = it_at(tokenz, arg_end);
						cur_it = skip_spaces_it(tokenz, cur_it);
						if (cur_it == tokenz.end()) return nullptr;
						// if comma, consume and continue
						if (cur_it->second->type() == token_type::comma)
						{
							cur_it = std::next(cur_it);
							cur_it = skip_spaces_it(tokenz, cur_it);
							continue;
						}
						// if closing ')', done
						if (cur_it->second->type() == token_type::expression_bound && cur_it->second->value == L")")
						{
							it_after_lparen = cur_it;
							break;
						}
						// otherwise parse error
						return nullptr;
					}
				}
				
				// Validate argument count and create appropriate expression
				if (func_entry->arity == 1)
				{
					if (args.size() != 1)
					{
						throw std::runtime_error("Function " + std::string(name.begin(), name.end()) + " expects 1 argument, got " + std::to_string(args.size()));
					}
					std::unique_ptr<unary_function_expression> uf = std::make_unique<unary_function_expression>();
					uf->func = func_entry->unary_func;
					uf->deriv_func = func_entry->unary_deriv;
					uf->arg = std::move(args[0]);
					
					token_map_iterator it_after = std::next(it_after_lparen);
					if (it_after == tokenz.end()) advance = it_after_lparen->first;
					else advance = it_after->first;
					return uf;
				}
				else if (func_entry->arity == 2)
				{
					if (args.size() != 2)
					{
						throw std::runtime_error("Function " + std::string(name.begin(), name.end()) + " expects 2 arguments, got " + std::to_string(args.size()));
					}
					std::unique_ptr<binary_function_expression> bf = std::make_unique<binary_function_expression>();
					bf->func = func_entry->binary_func;
					bf->deriv_func_arg1 = func_entry->binary_deriv_arg1;
					bf->deriv_func_arg2 = func_entry->binary_deriv_arg2;
					bf->arg1 = std::move(args[0]);
					bf->arg2 = std::move(args[1]);
					
					token_map_iterator it_after = std::next(it_after_lparen);
					if (it_after == tokenz.end()) advance = it_after_lparen->first;
					else advance = it_after->first;
					return bf;
				}
				else
				{
					throw std::runtime_error("Unsupported function arity");
				}
			}
			
			// Not a function call - check if it's a built-in constant
			if (constant_registry::instance().has(name))
			{
				std::unique_ptr<number_constant_expression> c = std::make_unique<number_constant_expression>();
				c->number = constant_registry::instance().get(name);
				if (it_next == tokenz.end()) advance = it->first;
				else advance = it_next->first;
				return c;
			}
			
			// Not a constant - it's a variable
			std::unique_ptr<variable_expression> var = std::make_unique<variable_expression>(name);
			variables_.push_back(var.get());  // track for root assignment
			
			if (it_next == tokenz.end()) advance = it->first;
			else advance = it_next->first;
			return var;
		}

		// parenthesis
		std::unique_ptr<expression> sub = compile_subexpression(tokenz, it->first, advance);
		if (sub) return sub;

		// number
		std::unique_ptr<number_constant_expression> num = compile_constant_number(tokenz, it->first, advance);
		if (num) return num;

		return nullptr;
	}

	// power: primary (** power)*  (right-associative)
	std::unique_ptr<expression> compile_power(const token_map& tokenz, size_t start_pos, size_t& advance)
	{
		size_t cur = start_pos;
		std::unique_ptr<expression> left = compile_primary(tokenz, cur, cur);
		if (!left) { advance = start_pos; return nullptr; }

		while (true)
		{
			token_map_iterator it = it_at(tokenz, cur);
			it = skip_spaces_it(tokenz, it);
			if (it == tokenz.end()) break;
			if (it->second->type() != token_type::binary_operator) break;
			if (it->second->value != L"**") break;
			// consume '**'
			token_map_iterator it_after = std::next(it);
			it_after = skip_spaces_it(tokenz, it_after);
			if (it_after == tokenz.end()) return nullptr;
			// right-associative: parse right as power
			size_t right_start = it_after->first;
			size_t right_end = right_start;
			std::unique_ptr<expression> right = compile_power(tokenz, right_start, right_end);
			if (!right) return nullptr;
			std::unique_ptr<binary_expression> p = std::make_unique<binary_expression>();
			p->op = expresie_tokenizer::power;
			p->eval_func_discrete = [](long double a, long double b) { return std::powl(a, b); };
			p->left = std::move(left);
			p->right = std::move(right);
			left = std::move(p);
			cur = right_end;
		}
		advance = cur;
		return left;
	}

	// unary: [+|-] power  (unary binds looser than power, so we parse power first for operand)
	std::unique_ptr<expression> compile_unary(const token_map& tokenz, size_t start_pos, size_t& advance)
	{
		token_map_iterator it = it_at(tokenz, start_pos);
		it = skip_spaces_it(tokenz, it);
		if (it == tokenz.end()) { advance = start_pos; return nullptr; }

		if (it->second->type() == token_type::unary_operator)
		{
			std::wstring opv = it->second->value;
			// consume unary op
			token_map_iterator it_after = std::next(it);
			it_after = skip_spaces_it(tokenz, it_after);
			if (it_after == tokenz.end()) { advance = start_pos; return nullptr; }
			size_t operand_start = it_after->first;
			size_t operand_end = operand_start;
			// parse next as power (so power binds tighter)
			std::unique_ptr<expression> operand = compile_power(tokenz, operand_start, operand_end);
			if (!operand) return nullptr;
			if (opv == L"+")
			{
				advance = operand_end;
				return operand; // unary plus no-op
			}
			else if (opv == L"-")
			{
				std::unique_ptr<unary_expression> ue = std::make_unique<unary_expression>();
				ue->operand = std::move(operand);
				ue->op = unary_minus;
				ue->eval_func = [](long double a) { return -a; };
				advance = operand_end;
				return ue;
			}
			else
			{
				advance = start_pos;
				return nullptr;
			}
		}
		// no unary operator -> parse power directly
		return compile_power(tokenz, it->first, advance);
	}

	// multiplicative: unary ((*|/) unary)*
	std::unique_ptr<expression> compile_multiplicative(const token_map& tokenz, size_t start_pos, size_t& advance)
	{
		size_t cur = start_pos;
		std::unique_ptr<expression> left = compile_unary(tokenz, cur, cur);
		if (!left) { advance = start_pos; return nullptr; }

		while (true)
		{
			token_map_iterator it = it_at(tokenz, cur);
			it = skip_spaces_it(tokenz, it);
			if (it == tokenz.end()) break;
			if (it->second->type() != token_type::binary_operator) break;
			std::wstring opv = it->second->value;
			if (opv != L"*" && opv != L"/") break;
			// consume op
			token_map_iterator it_after = std::next(it);
			it_after = skip_spaces_it(tokenz, it_after);
			if (it_after == tokenz.end()) return nullptr;
			size_t right_start = it_after->first;
			size_t right_end = right_start;
			std::unique_ptr<expression> right = compile_unary(tokenz, right_start, right_end);
			if (!right) return nullptr;
			std::unique_ptr<binary_expression> b = std::make_unique<binary_expression>();
			if (opv == L"*")
			{
				b->op = expresie_tokenizer::multiply;
				b->eval_func_discrete = [](long double a, long double b) { return a * b; };
			}
			else
			{
				b->op = expresie_tokenizer::divide;
				b->eval_func_discrete = [](long double a, long double b) { return a / b; };
			}
			b->left = std::move(left);
			b->right = std::move(right);
			left = std::move(b);
			cur = right_end;
		}
		advance = cur;
		return left;
	}

	// additive: multiplicative ((+|-) multiplicative)*
	std::unique_ptr<expression> compile_additive(const token_map& tokenz, size_t start_pos, size_t& advance)
	{
		size_t cur = start_pos;
		std::unique_ptr<expression> left = compile_multiplicative(tokenz, cur, cur);
		if (!left) { advance = start_pos; return nullptr; }

		while (true)
		{
			token_map_iterator it = it_at(tokenz, cur);
			it = skip_spaces_it(tokenz, it);
			if (it == tokenz.end()) break;
			if (it->second->type() != token_type::binary_operator) break;
			std::wstring opv = it->second->value;
			if (opv != L"+" && opv != L"-") break;
			// consume op
			token_map_iterator it_after = std::next(it);
			it_after = skip_spaces_it(tokenz, it_after);
			if (it_after == tokenz.end()) return nullptr;
			size_t right_start = it_after->first;
			size_t right_end = right_start;
			std::unique_ptr<expression> right = compile_multiplicative(tokenz, right_start, right_end);
			if (!right) return nullptr;
			std::unique_ptr<binary_expression> b = std::make_unique<binary_expression>();
			if (opv == L"+")
			{
				b->op = expresie_tokenizer::plus;
				b->eval_func_discrete = [](long double a, long double b) { return a + b; };
			}
			else
			{
				b->op = expresie_tokenizer::minus;
				b->eval_func_discrete = [](long double a, long double b) { return a - b; };
			}
			b->left = std::move(left);
			b->right = std::move(right);
			left = std::move(b);
			cur = right_end;
		}
		advance = cur;
		return left;
	}
};

}

#endif