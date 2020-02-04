#include "index.h"
#include <iomanip>
#include <iostream>
#include <sstream>


using namespace std;
static const int LINELEN(256);


// Speedup of only parsing for 4 instead of 7 fields is roughly 10ms with a
// total runtime of 160ms on this system (Core 2 Duo E6600, 2.4Ghz). 
Index::Index(const string filename, bool unique_origin, bool need_descpath,
			bool need_maintainer, bool need_categories) 
	: _file(filename.c_str(), ifstream::in), _unique_origin(unique_origin)
{
	_fields_num = need_categories ? 7 : ( need_maintainer ? 6 : (need_descpath ? 5 : 4));
}


Index::~Index() {}


bool Index::readable() {
	return _file.good();
}


bool Index::read_line() {
	getline(_file, _line);
	return _file.good();
}


bool Index::parse_line() {
	string::size_type oldposition = 0;
	string::size_type position;
		
	for (int i = 0; i < _fields_num; ++i) {
		position = _line.find('|', oldposition);
		if (position == string::npos) { return false; }
		_fields[i] = _line.substr(oldposition, position - oldposition);
		oldposition = position + 1;
	}
	
	_fields[Origin] = _fields[Origin].substr(11); // strip leading "/usr/ports/"

	return true;
}


set<string> Index::categories() {
	set<string> categories;
	string buf;
	stringstream ss(_fields[Categories]);
	
	while (ss >> buf) {
		categories.insert(buf);
	}
	
	return categories;
}


void Index::print_line(bool flag_name, bool flag_maintainer, bool flag_long) {
	if (_unique_origin) {
		if (_origins.find(_fields[Origin]) == _origins.end()) {
			_origins.insert(_fields[Origin]);
		} else {
			return;
		}
	}
	
	unsigned int firstfield = (flag_name ? Pkgname : Origin); 
	
	if (flag_maintainer) {
	    cout << left << setw(39) << _fields[firstfield];
	    cout << ' ' << _fields[Maintainer] << endl;
	} else {
	    cout << left << setw(25) << _fields[firstfield];
	    cout << ' ' << _fields[Desc] << endl;
	}
	
	if (flag_long) {
		char linebuf[LINELEN];
		ifstream pkg_descr(_fields[Descpath].c_str(), ifstream::in);
		
		while (pkg_descr.good()) {
			pkg_descr.getline(linebuf, LINELEN);
			cout << "    " << linebuf << endl;
		}
	}
}
