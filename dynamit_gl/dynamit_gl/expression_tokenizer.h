#pragma once
#ifndef __EXPRESSION_TOKENIZER_H__
#define __EXPRESSION_TOKENIZER_H__
#include <iostream>
#include <cassert>
#include "syntax_tree.h"

namespace expresie_tokenizer
{
template <typename T> inline bool in(T val) { return false; }
template <typename T, typename T2> inline bool in(T val, T2 arg) { return val == arg; }
template <typename T, typename T2, typename ... Ts> inline bool in(T val, T2 arg, Ts ... args)
{
	if (val == arg) return true;
	return in(val, args...);
}


enum class token_type : int
{
	unknown,
	number,
	space,
	identifier,
	keyword,
	op,               //operator
	unary_operator,   //unary operator (prefix)
	binary_operator,  //binary operator
	expression_bound, //it is ( or )
	comma,            // comma ','
	str,              //quoted string
	comment,
	end
};
inline std::wostream& operator<<(std::wostream& os, token_type t)
{
	switch (t)
	{
	case token_type::unknown:          return os << L"unknown";
	case token_type::number:           return os << L"number";
	case token_type::space:            return os << L"space";
	case token_type::identifier:       return os << L"identifier";
	case token_type::keyword:          return os << L"keyword";
	case token_type::op:               return os << L"op";
	case token_type::unary_operator:   return os << L"unary_operator";
	case token_type::binary_operator:  return os << L"binary_operator";
	case token_type::expression_bound: return os << L"expression_bound";
	case token_type::comma:            return os << L"comma";
	case token_type::str:              return os << L"str";
	case token_type::comment:          return os << L"comment";
	case token_type::end:              return os << L"end";
	}
	return os;
}

enum class token_result
{
	accept,
	reject, // completely unexpected input
	empty,  // zero length content, no spaces, no printable/unprintable characters
	finish,
	eos     // end of string
};

inline std::wostream& operator<<(std::wostream& os, token_result r)
{
	switch (r)
	{
	case token_result::accept: return os << L"accept";
	case token_result::reject: return os << L"reject";
	case token_result::empty:  return os << L"empty";
	case token_result::finish: return os << L"finish";
	case token_result::eos:    return os << L"eos";
	}
	return os;
}

class token {
public:
	std::wstring value;
	virtual token_type type()
	{
		return token_type::unknown;
	}
};

class number_token           : public token { public: virtual token_type type() { return token_type::number; } };
class space_token            : public token { public: virtual token_type type() { return token_type::space; } };
class identifier_token       : public token { public: virtual token_type type() { return token_type::identifier; } };
class unary_operator_token   : public token { public: virtual token_type type() { return token_type::unary_operator; } };
class binary_operator_token  : public token { public: virtual token_type type() { return token_type::binary_operator; } };
class expression_bound_token : public token { public: virtual token_type type() { return token_type::expression_bound; } };
class comma_token            : public token { public: virtual token_type type() { return token_type::comma; } };
class unknown_token          : public token { public: virtual token_type type() { return token_type::unknown; } };
class end_token              : public token { public: virtual token_type type() { return token_type::end; } };

inline std::wostream& operator<< (std::wostream& os, token& tk)
{
	os << tk.type();
	if (!tk.value.empty())
		os << L":" << tk.value;
	return os;
}
class tokenizer_base
{
public:
	tokenizer_base() {}
	virtual void         restart() { get_token().value = L""; }
	virtual token*       flush_result() = 0;
	virtual token&       get_token() = 0;
	virtual token_result check(wchar_t c) = 0;
	virtual void         feed(wchar_t c) = 0;
	virtual token_result validate() = 0;
};

template<typename T> class tokenizer : public tokenizer_base
{
public:
	std::wstring expected;
	std::unique_ptr<T> tkn = std::make_unique<T>();
	virtual T& get_token() { return *tkn; }
	virtual T* flush_result()
	{
		T* ret = tkn.release();
		tkn = std::make_unique<T>();
		return ret;
	}
	virtual void feed(wchar_t c) { tkn->value += c; }
	virtual void set(std::wstring c) { tkn->value = c; }
	virtual void add(std::wstring c) { tkn->value += c; }
	virtual void expect(std::wstring _expected) {
		expected = _expected;
	}
	virtual token_result validate() { return token_result::accept; }
};

class unknown_tokenizer : public tokenizer <unknown_token>
{
public:
	unknown_tokenizer() {}
	token_result check(wchar_t c)
	{
		assert(false && "unknown tokenizer should not be used in real tokenization");
		return token_result::accept;
	}
};

class space_tokenizer : public tokenizer <space_token>
{
public:
	space_tokenizer() {}
	token_result check(wchar_t c)
	{
		if (iswspace(c))
		{
			feed(c);
			return token_result::accept;
		}
		if (tkn->value.length() == 0) return token_result::empty;
		return token_result::finish;
	}
};

class number_tokenizer : public tokenizer <number_token>
{
private:
    bool has_dot = false;
    bool has_exp = false;
    bool exp_sign_allowed = false;
    
public:
    number_tokenizer() {}
    
    void restart() override 
    {
        tokenizer<number_token>::restart();
        has_dot = false;
        has_exp = false;
        exp_sign_allowed = false;
    }
    
    token_result check(wchar_t c)
    {
        if (iswdigit(c))
        {
            feed(c);
            exp_sign_allowed = false;
            return token_result::accept;
        }
        
        if (c == L'.' && !has_dot && !has_exp)
        {
            has_dot = true;
            feed(c);
            exp_sign_allowed = false;
            return token_result::accept;
        }
        
        if ((c == L'e' || c == L'E') && !has_exp && !tkn->value.empty())
        {
            has_exp = true;
            exp_sign_allowed = true;
            feed(c);
            return token_result::accept;
        }
        
        if ((c == L'+' || c == L'-') && exp_sign_allowed)
        {
            feed(c);
            exp_sign_allowed = false;
            return token_result::accept;
        }
        
        if (!tkn->value.empty())
            return token_result::finish;
            
        return token_result::empty;
    }
    
    token_result validate() override
    {
        const std::wstring& val = tkn->value;
        
        if (val.empty())
            return token_result::reject;
        
        wchar_t last = val.back();
        if (last == L'e' || last == L'E' || 
            last == L'+' || last == L'-' || last == L'.')
            return token_result::reject;
        
        if (val == L".")
            return token_result::reject;
            
        return token_result::accept;
    }
};

// Renamed from identifier_tokenizer to function_tokenizer
class function_tokenizer : public tokenizer<identifier_token>
{
public:
	function_tokenizer() {}
	token_result check(wchar_t c)
	{
		if (tkn->value.empty())
		{
			if (iswalpha(c) || c == L'_') { feed(c); return token_result::accept; }
			return token_result::empty;
		}
		if (iswalpha(c) || iswdigit(c) || c == L'_') { feed(c); return token_result::accept; }
		return token_result::finish;
	}
	token_result validate() override
	{
		if (tkn->value.empty()) return token_result::reject;
		return token_result::accept;
	}
};

class unary_operator_tokenizer : public tokenizer<unary_operator_token>
{
public:
	syntax_tree t = syntax_tree::merge<std::wstring>(L"+", L"-");
	
	unary_operator_tokenizer() {}
	
	token_result check(wchar_t c)
	{
		if (tkn->value.empty() && t.in_alphabet(c))
		{
			feed(c);
			return token_result::accept;
		}
		if (!tkn->value.empty())
			return token_result::finish;
		return token_result::empty;
	}
	
	token_result validate() override
	{
		if (tkn->value.empty())
			return token_result::reject;
		syntax_tree::search_result res = t.find(tkn->value);
		if (!res.found)
			return token_result::reject;
		return token_result::accept;
	}
};

class binary_operator_tokenizer : public tokenizer <binary_operator_token>
{
public:
	syntax_tree t = syntax_tree::merge <std::wstring>(L"+", L"-", L"*", L"**", L"/", L"%", L"==");
	binary_operator_tokenizer() {}
	token_result check(wchar_t c)
	{
		if (t.in_alphabet(c))
		{
			feed(c);
			return token_result::accept;
		}
		return token_result::finish;
	}

	virtual token_result validate()
	{
		std::unique_ptr<binary_operator_token> x = nullptr;
		x.reset(flush_result());
		syntax_tree::search_result res = t.find(x->value);
		if (!(res.found || res.partial_finish))  return token_result::reject;
		set(x->value.substr(0, res.iterated));
		return token_result::accept;
	}
};

class comma_tokenizer : public tokenizer<comma_token>
{
public:
	comma_tokenizer() {}
	token_result check(wchar_t c)
	{
		if (c == L',') { feed(c); return token_result::accept; }
		if (tkn->value.empty()) return token_result::empty;
		return token_result::finish;
	}
	token_result validate() override
	{
		if (tkn->value == L",") return token_result::accept;
		return token_result::reject;
	}
};

class expression_bound_tokenizer : public tokenizer <expression_bound_token>
{
public:
	syntax_tree syntax_tree = syntax_tree::merge <std::wstring>(L"(", L")");
	expression_bound_tokenizer() {}
	std::wstring accumulator;
	token_result check(wchar_t c)
	{
		if (syntax_tree.in_alphabet(c))
		{
			accumulator += c;
			syntax_tree::search_result res = syntax_tree.find(accumulator);
			if (res.found)
				feed(c);
			return token_result::accept;
		}
		return token_result::finish;
	}

	virtual token_result validate()
	{
		accumulator = L"";
		std::unique_ptr<expression_bound_token> x = nullptr;
		x.reset(flush_result());
		syntax_tree::search_result res = syntax_tree.find(x->value);
		if (res.final)
		{
			if (expected.length() > 0)
			{
				if (expected == x->value)
				{
					set(x->value);
					return token_result::accept;
				}
				else
					return token_result::reject;
			}
		}
		return token_result::reject;
	}
};


class expression_token_reader
{
public:

	token_result tokenize(const wchar_t* source, tokenizer_base* tkz, size_t& pos, size_t length)
	{
		size_t i = pos;
		token_result r = token_result::eos;
		tkz->restart();

		while (i < length)
		{
			r = tkz->check(source[i]);
			if (r != token_result::accept)
			{
				if (i == pos)
					r = token_result::empty;
				break;
			}
			i++;
		}

		if (r != token_result::reject)
		{
			r = tkz->validate();
			if (r != token_result::reject)
			{
				i = tkz->get_token().value.length();
				if (i == 0) return token_result::empty;
				pos += i;
			}
		}
		return r;
	}

	token_result tokenize_eat_spaces(const wchar_t* src, std::map<size_t, std::unique_ptr<token>>& tokenz, size_t& start, size_t length)
	{
		if (start == length) return token_result::empty;
		space_tokenizer tk_space;

		token_result tk_result;
		size_t advance = start;
		size_t save_advance = advance;

		tk_result = tokenize(src, &tk_space, advance, length);
		if (tk_result != token_result::empty)
		{
			start = advance;
			tokenz[save_advance].reset(tk_space.flush_result());
		}
		return tk_result;
	}

	token_result tokenize_eat_number(const wchar_t* src, std::map<size_t, std::unique_ptr<token>>& tokenz, size_t& start, size_t length)
	{
		if (start == length) return token_result::empty;
		number_tokenizer tk_number;

		token_result tk_result = token_result::empty;
		size_t advance = start;
		size_t save_advance = advance;

		tk_result = tokenize(src, &tk_number, advance, length);
		if (tk_result != token_result::empty)
		{
			start = advance;
			tokenz[save_advance].reset(tk_number.flush_result());
		}

		return tk_result;
	}
	
	token_result tokenize_eat_function(const wchar_t* src, std::map<size_t, std::unique_ptr<token>>& tokenz, size_t& start, size_t length)
	{
		if (start == length) return token_result::empty;
		function_tokenizer tk_func;

		token_result tk_result = token_result::empty;
		size_t advance = start;
		size_t save_advance = advance;

		tk_result = tokenize(src, &tk_func, advance, length);
		if (tk_result != token_result::empty && tk_result != token_result::reject)
		{
			start = advance;
			tokenz[save_advance].reset(tk_func.flush_result());
		}
		return tk_result;
	}
	
	token_result tokenize_eat_binary_operator(const wchar_t* src, std::map<size_t, std::unique_ptr<token>>& tokenz, size_t& start, size_t length)
	{
		if (start == length) return token_result::empty;
		binary_operator_tokenizer tk_binary_operator;

		token_result tk_result = token_result::empty;
		size_t advance = start;

		size_t save_advance = advance;
		tk_result = tokenize(src, &tk_binary_operator, advance, length);
		if (tk_result == token_result::reject) return tk_result;
		if (tk_result != token_result::empty)
		{
			start = advance;
			tokenz[save_advance].reset(tk_binary_operator.flush_result());
		}
		return tk_result;
	}

	token_result tokenize_eat_comma(const wchar_t* src, std::map<size_t, std::unique_ptr<token>>& tokenz, size_t& start, size_t length)
	{
		if (start == length) return token_result::empty;
		comma_tokenizer tk_comma;

		token_result tk_result = token_result::empty;
		size_t advance = start;
		size_t save_advance = advance;

		tk_result = tokenize(src, &tk_comma, advance, length);
		if (tk_result != token_result::empty && tk_result != token_result::reject)
		{
			start = advance;
			tokenz[save_advance].reset(tk_comma.flush_result());
		}
		return tk_result;
	}
	
	token_result tokenize_eat_grouping_parenthesis(const wchar_t* src, std::map<size_t, std::unique_ptr<token>>& tokenz, size_t& start, size_t length)
	{
		if (start == length) return token_result::empty;
		if (src[start] != L'(') return token_result::empty;
		
		expression_bound_tokenizer tk_bound;
		size_t advance = start;

		tk_bound.expect(L"(");
		token_result tk_result = tokenize(src, &tk_bound, advance, length);
		if (tk_result == token_result::reject)
			return token_result::empty;
		
		tokenz[start].reset(tk_bound.flush_result());

		size_t expr_start = advance;
		tokenize_expression(src, tokenz, advance, length);
		if (expr_start == advance)
			return token_result::reject;

		tk_bound.expect(L")");
		size_t close_pos = advance;
		tk_result = tokenize(src, &tk_bound, advance, length);
		if (tk_result == token_result::reject)
			return token_result::reject;
		
		tokenz[close_pos].reset(tk_bound.flush_result());
		start = advance;
		return token_result::accept;
	}
	
	token_result tokenize_eat_function_call(const wchar_t* src, std::map<size_t, std::unique_ptr<token>>& tokenz, size_t& start, size_t length)
	{
		if (start == length) return token_result::empty;
		
		size_t advance = start;
		size_t id_pos = advance;
		token_result id_result = tokenize_eat_function(src, tokenz, advance, length);
		if (id_result == token_result::empty || id_result == token_result::reject)
			return token_result::empty;
		
		tokenize_eat_spaces(src, tokenz, advance, length);
		
		if (advance >= length || src[advance] != L'(')
		{
			tokenz.erase(id_pos);
			return token_result::empty;
		}
		
		expression_bound_tokenizer tk_bound;
		
		tk_bound.expect(L"(");
		size_t open_pos = advance;
		token_result tk_result = tokenize(src, &tk_bound, advance, length);
		if (tk_result == token_result::reject)
		{
			tokenz.erase(id_pos);
			return token_result::empty;
		}
		tokenz[open_pos].reset(tk_bound.flush_result());
		
		tokenize_eat_spaces(src, tokenz, advance, length);
		
		if (advance < length && src[advance] == L')')
		{
			tk_bound.expect(L")");
			size_t close_pos = advance;
			tk_result = tokenize(src, &tk_bound, advance, length);
			if (tk_result == token_result::reject)
				return token_result::reject;
			tokenz[close_pos].reset(tk_bound.flush_result());
			start = advance;
			return token_result::accept;
		}
		
		size_t arg_start = advance;
		tokenize_expression(src, tokenz, advance, length);
		if (arg_start == advance)
			return token_result::reject;
		
		while (true)
		{
			tokenize_eat_spaces(src, tokenz, advance, length);
			
			token_result comma_result = tokenize_eat_comma(src, tokenz, advance, length);
			if (comma_result != token_result::accept)
				break;
			
			tokenize_eat_spaces(src, tokenz, advance, length);
			arg_start = advance;
			tokenize_expression(src, tokenz, advance, length);
			if (arg_start == advance)
				return token_result::reject;
		}
		
		tk_bound.expect(L")");
		size_t close_pos = advance;
		tk_result = tokenize(src, &tk_bound, advance, length);
		if (tk_result == token_result::reject)
			return token_result::reject;
		tokenz[close_pos].reset(tk_bound.flush_result());
		
		start = advance;
		return token_result::accept;
	}

	token_result tokenize_eat_primary(const wchar_t* src, std::map<size_t, std::unique_ptr<token>>& tokenz, size_t& start, size_t length)
	{
		size_t advance = start;
		
		token_result result = tokenize_eat_grouping_parenthesis(src, tokenz, advance, length);
		if (result == token_result::accept)
		{
			start = advance;
			return token_result::accept;
		}
		if (result == token_result::reject)
			return token_result::reject;
		
		result = tokenize_eat_number(src, tokenz, advance, length);
		if (result == token_result::accept || result == token_result::finish)
		{
			start = advance;
			return token_result::accept;
		}
		
		result = tokenize_eat_function_call(src, tokenz, advance, length);
		if (result == token_result::accept)
		{
			start = advance;
			return token_result::accept;
		}
		if (result == token_result::reject)
			return token_result::reject;
		
		result = tokenize_eat_function(src, tokenz, advance, length);
		if (result == token_result::accept || result == token_result::finish)
		{
			start = advance;
			return token_result::accept;
		}
		
		return token_result::empty;
	}

	token_result tokenize_eat_unary_expression(const wchar_t* src, std::map<size_t, std::unique_ptr<token>>& tokenz, size_t& start, size_t length)
	{
		size_t advance = start;
		
		bool found_unary = false;
		if (advance < length && (src[advance] == L'+' || src[advance] == L'-'))
		{
			std::unique_ptr<unary_operator_token> tok = std::make_unique<unary_operator_token>();
			tok->value = std::wstring(1, src[advance]);
			tokenz[advance] = std::move(tok);
			advance++;
			found_unary = true;
		}

		tokenize_eat_spaces(src, tokenz, advance, length);
		
		size_t primary_start = advance;
		token_result result = tokenize_eat_primary(src, tokenz, advance, length);
		
		if (result == token_result::reject)
			return token_result::reject;
		
		if (advance == primary_start)
		{
			if (found_unary)
				return token_result::reject;
			return token_result::empty;
		}
		
		start = advance;
		return token_result::accept;
	}

	void tokenize_expression(const wchar_t* src, std::map<size_t, std::unique_ptr<token>>& tokenz, size_t& start, size_t length)
	{
		using std::wcerr;
		using std::endl;

		token_result tk_result;
		size_t advance = start;

		tokenize_eat_spaces(src, tokenz, advance, length);

		bool haveExpression = false;

		tk_result = tokenize_eat_unary_expression(src, tokenz, advance, length);
		if (tk_result != token_result::reject && tk_result != token_result::empty)
			haveExpression = true;

		tokenize_eat_spaces(src, tokenz, advance, length);

		if (!haveExpression) { wcerr << L"{expected expression:" << (src + advance) << L"}" << endl; }
		else
			while (advance < length)
			{
				size_t local_start = advance;
				size_t before_binary = advance;

				tk_result = tokenize_eat_binary_operator(src, tokenz, advance, length);
				if (tk_result == token_result::reject) break;
				if (advance == length) break;
				if (tk_result == token_result::empty) break;

				size_t after_binary = advance;
				token_result space_result = tokenize_eat_spaces(src, tokenz, advance, length);
				
				if (advance == length) break;

				if (space_result == token_result::empty)
				{
					if (advance < length && (src[advance] == L'+' || src[advance] == L'-'))
					{
						std::map<size_t, std::unique_ptr<token>>::iterator it = tokenz.find(before_binary);
						if (it != tokenz.end() && it->second && it->second->value != L"**")
						{
							wcerr << L"        ERROR: operators must be separated by space: '"
								  << it->second->value << L"' and '" << src[advance] << L"' at position " << advance << endl;
							break;
						}
					}
				}

				tk_result = tokenize_eat_unary_expression(src, tokenz, advance, length);
				if (tk_result == token_result::reject || tk_result == token_result::empty)
				{
					wcerr << L"        ERROR: expected expression: {" << (src + advance) << L"}" << endl;
					break;
				}

				tokenize_eat_spaces(src, tokenz, advance, length);
				if (advance == length) break;

				if (local_start == advance) break;
			}
		start = advance;
	}

	std::map<size_t, std::unique_ptr<token>> tokenize_main(const wchar_t* src, bool show_results = false)
	{
		if (show_results)std::wcout << L"*****  test string: {" << src << L"}" << std::endl;
		using std::wcerr;
		using std::endl;
		std::map<size_t, std::unique_ptr<token>> tokenz;
		size_t advance = 0;
		size_t length = wcslen (src);

		tokenize_expression(src, tokenz, advance, length);

		if (advance < length)
		{
			std::wstring wstr = src + advance;
			unknown_tokenizer tk_unknown;
			tk_unknown.set(wstr);
			tokenz[advance].reset(tk_unknown.flush_result());
		}
		else
		{
			tokenz[advance].reset(std::make_unique<end_token>().release());
		}
		if (show_results)
		{
			wcerr << L"    -- tokens start --" << endl;
			for (std::map<size_t, std::unique_ptr<token>>::iterator it = tokenz.begin(); it != tokenz.end(); ++it)
				wcerr << L"        pos:" << it->first << L" token: {" << *it->second << L"}" << endl;
		}
		return tokenz;
	}
	std::map<size_t, std::unique_ptr<token>> tokenize_main(const std::wstring& src, bool show_results = false)
	{
		return tokenize_main(src.c_str(), show_results);
	}

};

}
#endif