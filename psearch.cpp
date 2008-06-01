#include <assert.h>
#include <errno.h>
#include <iostream>
#include <regex.h>
#include <set>
#include <stdlib.h>
#include <string>
#include <limits.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#include "getopt.h"
#include "index.h"
#include "regexlist.h"

using namespace std;


void print_copyright();
void print_help(char* progname);
/**
 * Compares sets a and b to see whether they are disjoint.
 */
bool disjoint(const set<string>& a, const set<string>& b);
/**
 * Finds the default index filename depending on the OS version
 * we're running on.
 */
void set_default_index_filename(string &index_filename);

const string PROGNAME("psearch");
const string VERSION("2.0rc3");
const string COPYRIGHT("Copyright (C) 2006-2008");
const string AUTHOR("Benjamin Lutz (http://public.xdi.org/=Benjamin.Lutz)");

string index_filename("");

int main(int argc, char** argv) {
	bool flag_long = false;
	bool flag_maintainer = false;
	bool flag_name = false;
	bool flag_or = false;
	bool flag_search_long = false;
	set<string> categories;
	Regexlist patterns;
	Regexlist inverse_patterns;
	
	set_default_index_filename(index_filename);

	if (argc == 1) {
		print_help(argv[0]);
		return 0;
	}
	
	const struct option longopts[] = {
		{ "version",     no_argument,       NULL, 'V' },
		{ "help",        no_argument,       NULL, 'h' },
		{ "category",    required_argument, NULL, 'c' },
		{ "file",        required_argument, NULL, 'f' },
		{ "long",        no_argument,       NULL, 'l' },
		{ "maintainer",  no_argument,       NULL, 'm' },
		{ "name",        no_argument,       NULL, 'n' },
		{ "or",          no_argument,       NULL, 'o' },
		{ "search_long", no_argument,       NULL, 's' },
		{ "inverse",     required_argument, NULL, 'v' },
		{ NULL,          no_argument,       NULL, 0 }
	};
	
	while (int ch = getopt_long(argc, argv, "Vhc:f:lmnosv:", longopts, NULL)) {
		if (-1 == ch) { break; }
		switch (ch) {
			case 'V': print_copyright(); return 0;
			case 'h': print_help(argv[0]); return 0;
			case 'c': categories.insert(string(optarg)); break;
			case 'f': index_filename = optarg; break;
			case 'l': flag_long = true; break;
			case 'm': flag_maintainer = true; break;
			case 'n': flag_name = true; break;
			case 'o': flag_or = true; break;
			case 's': flag_search_long = true; break;
			case 'v': inverse_patterns.append(optarg); break;
			case '?':
			default: print_help(argv[0]); return 1;
		}
	}
	
	for (int i = optind; i < argc; ++i) {
		patterns.append(argv[i]);
	}
	
	if (index_filename.empty()) {
		cerr << "Error: cannot determine the default path of the index file. Please specify" << endl;
		cerr << "the path to the index file manually using the -f option." << endl;
		return 1;
	}
	
	Index index(index_filename, flag_long | flag_search_long, flag_maintainer, not categories.empty());
	if (not index.readable()) {
		cerr << "Error: cannot read index file \"" << index_filename << "\"." << endl;
		return 1;
	}
	
	// finished with initalization
	
	// Now, read each line from the index file and maybe match it
	while (index.read_line() and index.parse_line()) {
		// Filter by categories, if the user specified it.
		if (not categories.empty()) {
			if (disjoint(categories, index.categories())) { continue; }
		}
		
		// Filter by inverse patterns, if the user specified it
		if (not inverse_patterns.empty()) {
			bool inv_match = false;
			for (inverse_patterns.first(); not inverse_patterns.last(); inverse_patterns.next() ) {
				inv_match = (inverse_patterns.match(index.pkgname()) or
						inverse_patterns.match(index.desc()));
				if (not inv_match and flag_maintainer) {
					inv_match = inverse_patterns.match(index.maintainer());
				}
				if (not inv_match and flag_search_long) {
					inv_match = inverse_patterns.match_file(index.descpath());
				}
				if (inv_match) { break; }
			}
			
			if (inv_match) { continue; }
		}
		
		bool all_pattern_match = !flag_or;
		for (patterns.first(); not patterns.last(); patterns.next() ) {
			bool match = (patterns.match(index.pkgname()) or patterns.match(index.desc()));
			if (not match and flag_maintainer) {
				match = patterns.match(index.maintainer());
			}
			if (not match and flag_search_long) {
				match = patterns.match_file(index.descpath());
			}
			all_pattern_match = (flag_or ? (all_pattern_match | match) : (all_pattern_match & match));
		}
		
		if (all_pattern_match) { index.print_line(flag_name, flag_maintainer, flag_long); }
	}
	
	return 0;
}


void print_copyright() {
	cout << PROGNAME << ' ' << VERSION << endl;
	cout << COPYRIGHT << ' ' << AUTHOR << endl;
}


void print_help(char* prog_name) {
	cout << "usage: " << prog_name << " [options] PATTERN [PATTERN ...]" << endl;
	cout << endl <<
"Searches ports for PATTERN. PATTERN is a case-insensitive regular expression." << endl <<
"if there is more than one pattern, each of them is searched for. By default," << endl <<
"ports are shown that match all patterns, use -o to show ports that match at" << endl <<
"least one pattern. By default, the name and the short description are searched." << endl <<
"If you specify the -s option, then the long description is searched as well." << endl <<
endl <<
"options:" << endl <<
"  -V, --version        Show program's version number and exit." << endl <<
"  -h, --help           Show this help message and exit." << endl <<
"  -c CATEGORY, --category=CATEGORY" << endl <<
"                       Only search for ports in CATEGORY. Speeds up searching." << endl <<
"  -f FILE, --file=FILE Path to INDEX file. Default: \"" << index_filename << "\"" << endl <<
"  -l, --long           Display long description (pkg-descr file) for any match." << endl <<
"  -m, --maintainer     Display maintainer instead of the short description," << endl <<
"                       and also search the maintainer field." << endl <<
"  -n, --name           Print canonical name of a port, including its version." << endl <<
"  -o, --or             Search for ports that match any PATTERN." << endl <<
"  -s, --search_long    Search long descriptions (pkg-descr file). Slows down" << endl <<
"                       searching." << endl <<
"  -v INVERSE_PATTERN, --inverse=INVERSE_PATTERN" << endl <<
"                       Searches for ports that do not match a pattern. May be" << endl <<
"                       specified several times." << endl;
}


bool disjoint(const set<string>& a, const set<string>& b) {
	set<string>::const_iterator it_a, it_b;
	for (it_a = a.begin(); it_a != a.end(); ++it_a) {
		for (it_b = b.begin(); it_b != b.end(); ++it_b) {
			if (*it_a == *it_b) { return false; }
		}
	}
	return true;
}


void set_default_index_filename(string &index_filename) {
	// get kern.ostype sysctl to see whether we're running
	// on FreeBSD. If we're not, don't set a standard index
	// file path
	int mib[2] = { CTL_KERN, KERN_OSTYPE };
	char ostype[8];
	size_t len = sizeof(ostype);
	
	int retval = sysctl(mib, 2, ostype, &len, NULL, 0);
	ostype[len - 1] = '\0';
	if (retval != 0 && errno != ENOMEM) {
		perror("sysctl");
		exit(1);
	}
	
	if (0 != strncmp("FreeBSD", ostype, len)) { return; }
	
	// get kern.osrelease sysctl to see whether we want
	// INDEX, INDEX-5 or INDEX-6 etc.
	mib[1] = KERN_OSRELEASE;
	char osrelease[32];
	len = sizeof(osrelease);
	retval = sysctl(mib, 2, osrelease, &len, NULL, 0);
	osrelease[len - 1] = '\0';
	if (retval != 0 && errno != ENOMEM) {
		perror("sysctl");
		exit(1);
	}
	
	// get major version
	char* s = strchr(osrelease, '.');
	if (s == NULL) { return; }
	size_t major_version_str_len = (size_t)(s - osrelease) + 1;
	char major_version_str[major_version_str_len];
	strlcpy(major_version_str, osrelease, major_version_str_len);
	long major_version = strtol(major_version_str, NULL, 10);
	if (major_version == 0) { return; }
	
	index_filename = "/usr/ports/INDEX";
	if (major_version >= 5) {
		index_filename.append("-");
		index_filename.append(major_version_str);
	}
}
