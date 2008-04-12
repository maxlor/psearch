#ifndef REGEXLIST_H
#define REGEXLIST_H

#include <list>
#include <regex.h>
#include <string>

using namespace std;

/** 
 * Regexlist is a list of regex(3) regular expressions. There is a match()
 * method that can be used for checking a string against all stored regular
 * expressions.
 * 
 * Regexlist has an internal iterator that is used with the first(), next()
 * and match() methods.
 */
class Regexlist {
public:
	Regexlist();
	~Regexlist();
	
	/**
	 * Append a regular expression to the list. Case-insensitive regular
	 * expressions are used.
	 * 
	 * @param pattern The pattern to be turned into a regular expression and
	 *        added to the list.
	 * @return false if pattern is not a valid regular expression, true
	 *         otherwise.
	 */
	bool append(const char* pattern);
	
	/**
	 * Number of regular expressions in the list.
	 */
	unsigned int length() const;
	
	/**
	 * Whether there are expressions in the list or not.
	 */
	bool empty() const;
	
	/**
	 * Set the internal iterator to the first element.
	 */
	void first();
	
	/**
	 * Advance the internal iterator.
	 */
	void next();
	
	/**
	 * @return true if the internal iterator is at the last element.
	 */
	bool last();
	
	/**
	 * Matches the regex pointed to by the internal iterator against s.
	 * @pre !last()
	 * @return true if the regular expression pointed to by the internal
	 *         iterator matches against s, false otherwise.
	 * @param s Some string.
	 */
	bool match(const string s) const;
	
	/**
	 * Matches the regex pointed to by the internal iterator against the
	 * text of the file at filename.
	 * @return true if the regular expression pointed to by the internal
	 *         iterator matches against s, false otherwise. In particular,
	 *         if there was an error reading the file, false is returned.
	 * @param filename Some filename.
	 */
	bool match_file(const string filename) const;
	
	/**
	 * The human-readable error message for an append() error. If append()
	 * return false, the given pattern contained an invalid regular
	 * expression. This function will give a human-readable description of
	 * the error.
	 * 
	 * The error message only applies to the most recent append() call.
	 */
	string error();

private:
	list<regex_t> regexes;
	list<regex_t>::iterator it;
	int errorcode;
	unsigned int len;
};


inline unsigned int Regexlist::length() const { return len; }
inline bool Regexlist::empty() const { return len == 0; }
inline void Regexlist::first() { it = regexes.begin(); }
inline void Regexlist::next() { ++it; }
inline bool Regexlist::last() { return (it == regexes.end()); }


#endif
