#ifndef INDEX_H
#define INDEX_H

#include <fstream>
#include <set>
#include <string>


using namespace std;


class Index {
public:
	enum Fieldname {
		Pkgname = 0, Path, Prefix, Desc, Descpath, Maintainer, Categories
	}; 
	
	/**
	 * Opens the index file named in filename, so that it can be parsed
	 * through parse_line() calls later.
	 * 
	 * @param filename file to be opened as index file.
	 * @param need_descpath Whether the path to the description file needs to
	 *                      be parsed from the index file later.
	 * @param need_categories Whether the categories field needs to parsed
	 *                        from the index file later. Implies
	 *                        need_descpath.
	 */
	Index(const string filename, bool need_descpath, bool need_categories);
	~Index();
	
	/**
	 * Is the index file readable? Use this after creating the object to
	 * check whether opening the file in the constructor worked.
	 */
	bool readable();
	
	/**
	 * Read a line from the index file.
	 * 
	 * @return true if the line could be read, false otherwise.
	 */
	bool read_line();
	
	/**
	 * Parse a line from the index file.
	 * 
	 * @return true if the line could be parsed, false otherwise.
	 */
	bool parse_line();
	
	/**
	 * Specifies whether the path to the pkg-descr files are being read from
	 * the INDEX file. This is controlled with the corresponding constructor
	 * argument.
	 */
	bool descpath_available() const;
	
	/**
	 * Specifies whether the categories were being read from the INDEX file
	 * This is controlled with the corresponding constructor argument.
	 */
	bool categories_available() const;
	
	///@{
	/**
	 * Return a fieldname. The fields Maintainer() and Categories() are only
	 * available if need_categories was true in the constructor. Descpath()
	 * is only available if need_descpath or need_categories were true in the
	 * constructor.
	 */
	string pkgname() const;
	string path() const;
	string prefix() const;
	string desc() const;
	string descpath() const;
	string maintainer() const;
	///@}
	
	/**
	 * Categories this entry belongs to.
	 */
	set<string> categories();
	
	/**
	 * Prints the entry for the currently parsed line.
	 * 
	 * @param flag_name If true, print package name instead of path.
	 * @param flag_long If true, print pkg-descr file too.
	 */
	void print_line(bool flag_name, bool flag_long);

private:
	ifstream file;
	string line;
	bool need_descpath;
	bool need_categories;
	unsigned int fields_len;
	string fields[7];
	set<string> _categories;
};


inline bool Index::read_line() {
	getline(file, line);
	return file.good();
}


inline bool Index::descpath_available() const { return need_descpath; }
inline bool Index::categories_available() const { return need_categories; }
inline string Index::pkgname()    const { return fields[Pkgname]; }
inline string Index::path()       const { return fields[Path]; }
inline string Index::prefix()     const { return fields[Prefix]; }
inline string Index::desc()       const { return fields[Desc]; }
inline string Index::descpath()   const { return fields[Descpath]; }
inline string Index::maintainer() const { return fields[Maintainer]; }

#endif
