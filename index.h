#ifndef INDEX_H
#define INDEX_H

#include <fstream>
#include <set>
#include <string>


class Index {
public:
	enum Fieldname {
		Pkgname = 0, Origin, Prefix, Desc, Descpath, Maintainer, Categories
	}; 
	
	/**
	 * Opens the index file named in filename, so that it can be parsed
	 * through parse_line() calls later.
	 * 
	 * @param filename file to be opened as index file.
	 * @param unique_origin If true, a row which has the same origin as
	 *                      a previous row will be skipped.
	 * @param need_descpath Whether the path to the description file needs
	 *                      to be parsed from the index file later.
	 * @param need_maintainer Whether the maintainer field needs to be
	 *                        parsed from the index file later. Implies
	 *                        need_descpath.
	 * @param need_categories Whether the categories field needs to be
	 *                        parsed from the index file later. Implies
	 *                        need_maintainer and need_descpath.
	 */
	Index(const std::string filename, bool unique_origin,
		  bool need_descpath, bool need_maintainer, bool need_categories);
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
	
	///@{
	/**
	 * Return a fieldname. The fields maintainer() and categories() are only
	 * available if need_categories was true in the constructor. descpath()
	 * is only available if need_descpath or need_categories were true in the
	 * constructor.
	 */
	std::string pkgname() const;
	std::string origin() const;
	std::string prefix() const;
	std::string desc() const;
	std::string descpath() const;
	std::string maintainer() const;
	///@}
	
	/**
	 * Categories this entry belongs to.
	 */
	std::set<std::string> categories();
	
	/**
	 * Prints the entry for the currently parsed line.
	 * 
	 * @param flag_name If true, print package name instead of path.
	 * @param flag_maintainer If true, print the maintainer's email
	 *                        address instead of the package description.
	 * @param flag_long If true, print pkg-descr file too.
	 */
	void print_line(bool flag_name, bool flag_maintainer, bool flag_long);

private:
	std::ifstream _file;
	std::string _line;
	bool _unique_origin;
	unsigned int _fields_num;
	std::string _fields[7];
	std::set<std::string> _origins;
};


inline std::string Index::pkgname()    const { return _fields[Pkgname]; }
inline std::string Index::origin()     const { return _fields[Origin]; }
inline std::string Index::prefix()     const { return _fields[Prefix]; }
inline std::string Index::desc()       const { return _fields[Desc]; }
inline std::string Index::descpath()   const { return _fields[Descpath]; }
inline std::string Index::maintainer() const { return _fields[Maintainer]; }

#endif
