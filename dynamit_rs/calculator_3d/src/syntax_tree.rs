//! Syntax tree (trie) for operator and keyword matching.
//!
//! Port of C++ syntax_tree.h

use std::collections::{HashMap, HashSet};

/// Result of searching for a string in the syntax tree
#[derive(Debug, Clone, Default)]
pub struct SearchResult {
    /// Number of characters matched
    pub iterated: usize,
    /// True if the entire search string was found
    pub found: bool,
    /// True if current position is a valid end of a token
    pub is_final: bool,
    /// Position of last valid final match (for partial matches)
    pub partial_finish: usize,
}

/// Trie-based syntax tree for matching operators and keywords
#[derive(Debug, Clone, Default)]
pub struct SyntaxTree {
    children: HashMap<char, SyntaxTree>,
    alphabet: HashSet<char>,
    alphabet_starts: HashSet<char>,
    is_final: bool,
}

impl SyntaxTree {
    /// Create a new empty syntax tree
    pub fn new() -> Self {
        Self::default()
    }

    /// Create a syntax tree from multiple string patterns
    pub fn from_patterns<I, S>(patterns: I) -> Self
    where
        I: IntoIterator<Item = S>,
        S: AsRef<str>,
    {
        let mut tree = Self::new();
        let patterns: Vec<_> = patterns.into_iter().collect();
        for pattern in &patterns {
            tree.add_to_alphabet(pattern.as_ref());
        }
        for pattern in &patterns {
            tree.insert(pattern.as_ref());
        }
        tree
    }

    /// Create a syntax tree from a slice of string patterns
    pub fn merge(patterns: &[&str]) -> Self {
        let mut tree = Self::new();
        for pattern in patterns {
            tree.add_to_alphabet(pattern);
        }
        for pattern in patterns {
            tree.insert(pattern);
        }
        tree
    }

    /// Add all characters from a string to the alphabet
    fn add_to_alphabet(&mut self, s: &str) {
        for c in s.chars() {
            self.alphabet.insert(c);
        }
        if let Some(first) = s.chars().next() {
            self.alphabet_starts.insert(first);
        }
    }

    /// Insert a string pattern into the tree
    pub fn insert(&mut self, s: &str) {
        let chars: Vec<char> = s.chars().collect();
        let mut node = self;

        for (i, &c) in chars.iter().enumerate() {
            let is_last = i == chars.len() - 1;

            node = node.children.entry(c).or_insert_with(SyntaxTree::new);

            if is_last {
                node.is_final = true;
            }
        }
    }

    /// Search for a string in the tree
    pub fn find(&self, s: &str) -> SearchResult {
        self.find_from(s, 0)
    }

    /// Search for a string starting from a given position
    pub fn find_from(&self, s: &str, start: usize) -> SearchResult {
        let mut result = SearchResult::default();
        let mut node = self;

        for c in s.chars().skip(start) {
            match node.children.get(&c) {
                Some(child) => {
                    node = child;
                    result.iterated += 1;
                    result.is_final = node.is_final;
                    if node.is_final {
                        result.partial_finish = result.iterated;
                    }
                }
                None => return result,
            }
        }

        if result.iterated == s.chars().count() - start {
            result.found = true;
        }

        result
    }

    /// Check if a character is in the alphabet
    pub fn in_alphabet(&self, c: char) -> bool {
        self.alphabet.contains(&c)
    }

    /// Check if a character can start a valid token
    pub fn can_start(&self, c: char) -> bool {
        self.alphabet_starts.contains(&c)
    }

    /// Get the alphabet
    pub fn alphabet(&self) -> &HashSet<char> {
        &self.alphabet
    }

    /// Check if this node is a final state
    pub fn is_final(&self) -> bool {
        self.is_final
    }

    /// Check if the tree is empty
    pub fn is_empty(&self) -> bool {
        self.children.is_empty()
    }

    /// Get the number of children
    pub fn len(&self) -> usize {
        self.children.len()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_syntax_tree_basic() {
        let tree = SyntaxTree::merge(&["+", "-", "*", "**", "/"]);

        assert!(tree.in_alphabet('+'));
        assert!(tree.in_alphabet('*'));
        assert!(!tree.in_alphabet('x'));

        let result = tree.find("+");
        assert!(result.found);
        assert!(result.is_final);

        let result = tree.find("**");
        assert!(result.found);
        assert!(result.is_final);

        let result = tree.find("***");
        assert!(!result.found);
        assert_eq!(result.iterated, 2);
    }

    #[test]
    fn test_partial_match() {
        let tree = SyntaxTree::merge(&["++", "+"]);

        let result = tree.find("+");
        assert!(result.found);
        assert!(result.is_final);

        let result = tree.find("++");
        assert!(result.found);
        assert!(result.is_final);
    }
}
