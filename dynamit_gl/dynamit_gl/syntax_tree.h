#pragma once
#ifndef __SYNTAX_TREE_H__
#define __SYNTAX_TREE_H__
#include <map>
#include <set>
#include <string>
namespace expresie_tokenizer
{

class syntax_tree
{
public:
	struct search_result { size_t iterated = 0; bool found = false; bool final = false; size_t partial_finish = 0; };
	std::map<wchar_t, syntax_tree> children;
	std::set<wchar_t> alphabet;  // Cached alphabet - built once during merge()
	std::set<wchar_t> alphabet_starts;  // Good idea to have set of valid starting characters, immediate rejection of invalid tokens
	bool final = false;
	syntax_tree& operator[](wchar_t c) { return children[c]; }
	std::map<wchar_t, syntax_tree>::iterator begin() { return children.begin(); }
	std::map<wchar_t, syntax_tree>::iterator end() { return children.end(); }
	std::map<wchar_t, syntax_tree>::iterator find(wchar_t c) { return children.find(c); }
	bool empty() { return children.empty(); }
	size_t size() { return children.size(); }
	operator bool() { return final; } //empty tree

	template<typename T, typename... Args> static syntax_tree merge(Args&&... args)
	{
		syntax_tree tree;
		// Build alphabet from all characters in all arguments
		(tree.add_to_alphabet(std::forward<Args>(args)), ...);
		// Insert tokens into tree
		(tree.insert(std::forward<Args>(args)), ...);
		return tree;
	}

private:
	// Add all characters from a string to the alphabet
	void add_to_alphabet(const std::wstring& str)
	{
		for (wchar_t c : str)
		{
			alphabet.insert(c);
		}
		alphabet_starts.insert(str[0]);
	}

public:
	void insert(const std::wstring& str)
	{
		syntax_tree* node = this;
		for (size_t i = 0; i < str.length(); i++)
		{
			wchar_t c = str[i];
			bool finish = i == str.length() - 1;
			std::map<wchar_t, syntax_tree>::iterator it = node->find(c);
			if (it != node->end())
			{
				node = &it->second;
				if (finish) node->final = finish;
			}
			else
			{
				syntax_tree new_node;
				new_node.final = finish;
				(*node)[c] = new_node;
				node = &node->children[c];
			}
		}
	}

	search_result find(const std::wstring& str, size_t start = 0)
	{
		search_result sr = {};
		syntax_tree* node = this;
		size_t i = start;
		for (; i < str.length(); i++)
		{
			wchar_t c = str[i];
			std::map<wchar_t, syntax_tree>::iterator it = node->find(c);
			if (it == node->end()) return sr;
			//if (it == node->end()) break;  // almost same thing as return sr;

			node = &it->second;
			sr.iterated++;
			sr.final = node->final;
			if (node->final) sr.partial_finish = sr.final;
			if (sr.iterated == str.length())
				sr.found = true;

		}

		return sr;
	}

	// Get the cached alphabet (const reference - no copy)
	const std::set<wchar_t>& get_alphabet() const
	{
		return alphabet;
	}
	// Get the cached alphabet (const reference - no copy)
	const std::set<wchar_t>& get_alphabet_starts() const
	{
		return alphabet_starts;
	}

	// Check if a character is in the alphabet (valid first character) - uses cached set
	bool in_alphabet(wchar_t c) const
	{
		return alphabet.count(c)  > 0;
	}
	bool can_start(wchar_t c) const
	{
		return alphabet_starts.count(c)  > 0;
	}
};
}
#endif