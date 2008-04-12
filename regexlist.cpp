#include "regexlist.h"
#include <iostream>
#include <fstream>
#include <sys/file.h> // for flock()


static const int LINELEN(256);


Regexlist::Regexlist() {
	len = 0;
}


Regexlist::~Regexlist() {
	list<regex_t>::iterator regex_it;
	
	for (regex_it = regexes.begin(); regex_it != regexes.end(); ++regex_it) {
		regfree(&*regex_it);
	}
	regexes.clear();
}


bool Regexlist::append(const char* pattern) {
	regex_t regexp;
	
	int flags = REG_EXTENDED | REG_ICASE | REG_NOSUB;
	errorcode = regcomp(&regexp, pattern, flags);
	if (errorcode != 0) { return false; }
	
	regexes.push_back(regexp);
	
	++len;
	
	return true;
}


bool Regexlist::match(const string s) const {
	const char* c_str = s.c_str();
	return (0 == regexec(&*it, c_str, 0, NULL, 0));
}


bool Regexlist::match_file(const string filename) const {
	char linebuf[LINELEN];
	
	ifstream pkg_descr(filename.c_str(), ifstream::in);
	while (pkg_descr.good()) {
		pkg_descr.getline(linebuf, LINELEN);
		if (0 == regexec(&*it, linebuf, 0, NULL, 0)) { return true; }
	}
	return false;	
}


string Regexlist::error() {
	// This first call here only gets the necessary buffer length.
	const unsigned int buflen = regerror(errorcode, NULL, NULL, 0);
	char buffer[buflen];
	
	regerror(errorcode, NULL, buffer, buflen);
	return string(buffer);
}
