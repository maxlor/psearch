#include "index.h"
#include <iomanip>
#include <iostream>
#include <sstream>


static const int LINELEN(256);


// Speedup of only parsing for 4 instead of 7 fields is roughly 10ms with a
// total runtime of 160ms on this system (Core 2 Duo E6600, 2.4Ghz). 
Index::Index(const string filename, bool need_descpath, bool need_maintainer,
	bool need_categories) 
	: file(filename.c_str(), ifstream::in)
{
	//this->need_descpath   = need_categories | need_maintainer | need_descpath ;
	//this->need_maintainer = need_categories | need_maintainer;
	this->need_categories = need_categories;
	
	fields_num = need_categories ? 7 : ( need_maintainer ? 6 : (need_descpath ? 5 : 4));
}


Index::~Index() {}


bool Index::readable() {
	return file.good();
}


bool Index::parse_line() {
	string::size_type oldposition = 0;
	string::size_type position;
		
	for (int i = 0; i < fields_num; ++i) {
		position = line.find('|', oldposition);
		if (position == string::npos) { return false; }
		fields[i] = line.substr(oldposition, position - oldposition);
		oldposition = position + 1;
	}
	
	fields[Path] = fields[Path].substr(11); // strip leading '/usr/ports/'
	if (need_categories) { _categories.clear(); }
}


set<string> Index::categories() {
	if (_categories.empty()) {
		string buf;
		stringstream ss(fields[Categories]);
		
		while (ss >> buf) {
			_categories.insert(buf);
		}
	}
	return _categories;
}


void Index::print_line(bool flag_name, bool flag_maintainer, bool flag_long) {
	unsigned int firstfield = (flag_name ? Pkgname : Path); 
	
	if (flag_maintainer) {
	    cout << fields[firstfield] << setw(40 - fields[firstfield].length()) << ' ';
	    cout << fields[Maintainer] << endl;
	} else {
	    cout << fields[firstfield] << setw(26 - fields[firstfield].length()) << ' ';
	    cout << fields[Desc] << endl;
	}
	
	if (flag_long) {
		char linebuf[LINELEN];
		ifstream pkg_descr(fields[Descpath].c_str(), ifstream::in);
		
		while (pkg_descr.good()) {
			pkg_descr.getline(linebuf, LINELEN);
			cout << "    " << linebuf << endl;
		}
	}
}
