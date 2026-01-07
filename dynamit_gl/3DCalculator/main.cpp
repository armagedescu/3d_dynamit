#include "enabler.h"
#ifdef __MAIN_CPP__

#include <iostream>
#include <map>
#include <vector>
#include <cassert>

//#include "expression_tokenizer.h"
#include "expression_compiler.h"

namespace expresie_tokenizer {}


using namespace std;
using namespace expresie_tokenizer;


wostream& operator<<(wostream& os, const std::set<wchar_t>& s)
{
	for (auto c : s) os << c << L" ";
	return os;
}


int main()
{
	//assert(false && "salut");

	//std::string sm = sum<"1", "2", "3", "4", "5">();
	static_assert (1 == 1);


	cout << endl;
	syntax_tree word_sup_tree = syntax_tree::merge <std::wstring>(L"salut", L"salpetru", L"superior", L"super", L"cupru", L"cuplu", L"cunst", L"cunstler");
	std::wcout << L"alphabet chars:  " << word_sup_tree.get_alphabet() << endl;
	std::wcout << L"alphabet starts: " << word_sup_tree.get_alphabet_starts() << endl;

	cout << "word_sup_tree, tokens:   [cupru, cuplu,              super, superior,                  salut, salpetru, cunst, cunstler]" << endl;
	cout << "validate against inputs: [cupru, cuplu, cupra, cup,  super, superi, superior, supero,                   cunst, cunstl]" << endl;
	for (std::wstring sup_word : {L"cupru", L"cuplu", L"cupra", L"cup", L"super", L"superi", L"superior", L"supero", L"cunst", L"cunstl", L"cunsto", L"cunstlo"})
	{
		word_sup_tree.find(sup_word);
		syntax_tree::search_result res = word_sup_tree.find(sup_word);
		std::wcout << "   ";
		std::wcout << (res.found ? "+" : "!") << " found:" << sup_word << "; ";
		std::wcout << "final:" << (res.final ? "true" : "false") << "; ";
		if (!res.found) std::wcout << "partial:" << sup_word.substr(0, res.iterated) << "; ";
		std::wcout << std::endl;
	}



	syntax_tree operator_tree = syntax_tree::merge <std::wstring>(L"==", L"*", L"**", L"+", L"/", L"%");
	bool btr = operator_tree['='].final;
	wcout << "operator tree navigation, tokens [==, *, **, +, /, %]:" << endl;
	std::wcout << "   btr =    " << (operator_tree['='].final ? "true" : "false") << std::endl;
	std::wcout << "   btr =[=] " << (operator_tree['=']['='].final ? "true" : "false") << std::endl;
	std::wcout << "   btr *    " << (operator_tree['*'].final ? "true" : "false") << std::endl;
	std::wcout << "   btr **   " << (operator_tree['*']['*'].final ? "true" : "false") << std::endl;
	std::wcout << "   btr +    " << (operator_tree['+'].final ? "true" : "false") << std::endl;

	std::wcout << "iterate operator tree, first level:" << endl;
	for (auto& x : operator_tree) wcout << "     char: '" << x.first << "'" << std::endl;

	// Test alphabet extraction (cached)
	std::wcout << L"\nAlphabet extraction test (cached in syntax_tree):" << endl;
	const std::set<wchar_t>& op_alphabet = operator_tree.get_alphabet();
	std::wcout << L"  Operator alphabet: ";
	for (wchar_t c : op_alphabet) {
		std::wcout << L"'" << c << L"' ";
	}
	std::wcout << std::endl;

	// Test in_alphabet (uses cached set)
	std::wcout << L"  Is '+' in operator alphabet? " << (operator_tree.in_alphabet(L'+') ? L"Yes" : L"No") << std::endl;
	std::wcout << L"  Is 'x' in operator alphabet? " << (operator_tree.in_alphabet(L'x') ? L"Yes" : L"No") << std::endl;
	std::wcout << L"  Is '=' in operator alphabet? " << (operator_tree.in_alphabet(L'=') ? L"Yes" : L"No") << std::endl;

	//return 0;

	if (__is_base_of(token, number_token))
		std::cout << "number_token is derived from token" << std::endl;
	else
		std::cout << "number_token is NOT derived from token" << std::endl;
	expression_token_reader tr;
	//tr.tokenize_main(" ((1234 )) ++-*/ 234  456 ");
	//tr.tokenize_main("  ((1234)) +  ((((234))) **   (456) "); //good case
	//tr.tokenize_main("  ((   1234 + 3  * (2 * (3/2+5))   )) +  (((234))) **   (456 + 5) ");
	//tr.tokenize_main("  (((1234))) +  234 **   456 ");
	//tr.tokenize_main("1234  234  456   ");

	//std::string program_source = " 1234  + 234   ";
	std::wstring program_source;
	expression_token_compiler  compiler;
	std::unique_ptr<expression> program = nullptr;
	std::map<size_t, std::unique_ptr<token>> lang;

	program_source = L" 10 - 15 +15 -15-15 +10+10-15-15";
	program_source = L" 10 * 5 /2 /5 - 1 - 1 + 2 + 6";
	program_source = L"1+1";
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;

	program_source = L"5.5 - 9*3/3/3 + 6";
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;

	program_source = L"5.5 - 9*3/3/3 + -6";
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;

	program_source = L"5.5 - 9*3/3/3 + +6";
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;

	program_source = L"5.5 - 9*3/3/3 + +6 + 4**2";
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;
	program_source = L"5.5 - 9*3/3/3 + +6 + -4**2";
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;
	wcout << L"-----------------" << endl;

	program_source = L"5+ -3";
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;
	wcout << L"-----------------" << endl;

	program_source = L"5.5 - -9*3/3/3 + +6 + -(4**2)";
	program_source = L"5.5 - -9*3/3/3 + +6 + -(4**2)";
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;

	program_source = L"5.5 - -9*3/3/3 + +6 + -4**2";
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;

	program_source = L"5.5 - -9*3/3/3 + +6 + -4**2";
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;

	////program_source = " ((10 + 5*2) + 2 * 6 / 2) - 15  + 6 ";
	program_source = L" ((10 + 5*(2)) +  6 * 3 / 2) - 15  + 6 ";
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;
	program_source = L"  (10-4/(2*2) + + 3) ";
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;
	//
	//program_source = L" (10 + 5*(2)) +  6 * 3 / 2 / 2";
	////program_source = L"  8 ";
	//lang = tr.tokenize_main(program_source);
	//program = compiler.compile(lang);
	//wcout << "evaluate: " << program_source << " = " << program->eval() << endl;
	//
	program_source = L" ((10 + 5*(2)) +  6 * 3 / 2) - 15  + 6 *    (3 * (5-2) + 8/(10-4/(2*2) + 3) )"; //(10-4/(2*2) + 3)
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;

	program_source = L"sin (3.14159561 / (18/ -3))";
	lang = tr.tokenize_main(program_source);
	program = compiler.compile(lang);
	wcout << "evaluate: " << program_source << " = " << program->eval() << endl;

	long double x = 0, y = 0, z = 0;
	//variable_registry::instance().bind(L"x", &x);
	//variable_registry::instance().bind(L"y", &y);
	//variable_registry::instance().bind(L"z", &z);

	//program = compiler.compile(tr.tokenize_main(L"x*x + y*y + z*z"));
	program = compiler.compile(L"x*x + y*y + z*z");
	program->bind(L"x", &x);
	program->bind(L"y", &y);
	program->bind(L"z", &z);

	x = 3; y = 4; z = 0;
	std::wcout << program->eval() << std::endl;  // 25

	x = 1; y = 1; z = 1;
	std::wcout << program->eval() << std::endl;  // 3

	//expr.reset(compiler.compile(lang));
	return 0;
}

#endif