#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <regex>
#include <set>
#include <string>
#include <vector>
extern "C" {
#include <sys/types.h>
#ifdef __FreeBSD__
#include <sys/sysctl.h>
#include <getopt.h>
#endif
}

#include "index.h"

using namespace std;

void print_copyright();
void print_help(char* progname, const string &index_filename);
/**
 * Compares sets a and b to see whether they are disjoint.
 */
bool disjoint(const set<string>& a, const set<string>& b);

bool regex_search_file(const string &path, const regex &r);

/**
 * Finds the default index filename depending on the OS version
 * we're running on.
 */
void set_default_index_filename(string &index_filename);

const string PROGNAME("psearch");
const string VERSION("2.1.0");
const string COPYRIGHT("Copyright 2008, 2012, 2020");
const string AUTHOR("Benjamin Lutz (mail");
const string AUTHOR2("maxlor.com)");


int main(int argc, char** argv) {
	bool flag_long = false;
	bool flag_maintainer = false;
	bool flag_name = false;
	bool flag_or = false;
	bool flag_search_long = false;
	string index_filename;
	set<string> categories;
	vector<regex> patterns;
	vector<regex> inverse_patterns;
	const regex::flag_type regex_flags = regex::extended | regex::icase | regex::nosubs | regex::optimize;
	
	set_default_index_filename(index_filename);

	if (argc == 1) {
		print_help(argv[0], index_filename);
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
			case 'h': print_help(argv[0], index_filename); return 0;
			case 'c': categories.insert(string(optarg)); break;
			case 'f': index_filename = optarg; break;
			case 'l': flag_long = true; break;
			case 'm': flag_maintainer = true; break;
			case 'n': flag_name = true; break;
			case 'o': flag_or = true; break;
			case 's': flag_search_long = true; break;
			case 'v': inverse_patterns.push_back(regex(optarg, regex_flags)); break;
			case '?':
			default: print_help(argv[0], index_filename); return 1;
		}
	}
	
	for (int i = optind; i < argc; ++i) {
		patterns.push_back(regex(argv[i], regex_flags));
	}
	
	if (index_filename.empty()) {
		cerr << "Error: cannot determine the default path of the index file. Please specify" << endl;
		cerr << "the path to the index file manually using the -f option." << endl;
		return 1;
	}
	
	Index index(index_filename, !flag_name, flag_long | flag_search_long, flag_maintainer,
				not categories.empty());
	if (not index.readable()) {
		cerr << "Error reading index file \"" << index_filename << "\": "
			<< strerror(errno) << endl;
		return 1;
	}
	
	// finished with initalization
	
	// Now, read each line from the index file and maybe match it
	while (index.read_line()) {
		if (not index.parse_line()) { continue; }
		// Filter by categories, if the user specified it.
		if (not categories.empty()) {
			if (disjoint(categories, index.categories())) { continue; }
		}
		
		// Filter by inverse patterns, if the user specified it
		bool inv_match = false;
		for (const regex &r : inverse_patterns) {
			inv_match = regex_search(index.pkgname(), r)
				or regex_search(index.origin(), r)
				or regex_search(index.desc(), r)
				or (flag_maintainer and regex_search(index.maintainer(), r))
				or (flag_search_long and regex_search_file(index.descpath(), r));
			if (inv_match) { break; }
		}		
		if (inv_match) { continue; }
		
		// Filter by patterns
		bool match_one = false;
		bool match_all = true;
		for (const regex &r : patterns) {
			bool match = regex_search(index.pkgname(), r) 
					or regex_search(index.origin(), r)
					or regex_search(index.desc(), r)
					or (flag_maintainer and regex_search(index.maintainer(), r))
					or (flag_search_long and regex_search_file(index.descpath(), r));
			match_one |= match;
			match_all &= match;
			
			if (flag_or) {
				if (match_one) { break; }
			} else if (not match_all) {
				break;
			}
		}
		
		if (match_all or (flag_or and match_one)) {
			index.print_line(flag_name, flag_maintainer, flag_long);
		}
	}
	
	return 0;
}


void print_copyright() {
	cout << PROGNAME << ' ' << VERSION << endl;
	cout << COPYRIGHT << ' ' << AUTHOR << '@' << AUTHOR2 << endl;
}


void print_help(char* prog_name, const string &index_filename) {
	cout << "Usage: " << prog_name << R"( [options] PATTERN [PATTERN ...]

Lists ports whose description matches PATTERN. 

Options:
  -V, --version        Show program's version number and exit.
  -h, --help           Show this help message and exit.
  -c CATEGORY, --category=CATEGORY
                       Only search for ports in CATEGORY.
  -f FILE, --file=FILE Path to INDEX file. Default: ")" << index_filename << R"(".
  -l, --long           Display long description (pkg-descr file) for any match.
  -m, --maintainer     Search and display the maintainer.
  -n, --name           Print canonical name of a port, including its version.
  -o, --or             Search for ports that match any PATTERN.
  -s, --search_long    Search long descriptions (pkg-descr file). Slow.
  -v INVERSE_PATTERN, --inverse=INVERSE_PATTERN
                       Searches for ports that do not match a pattern.
)";
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


bool regex_search_file(const string &path, const regex &r) {
	string line;
	
	ifstream file(path);
	while (file.good()) {
		getline(file, line);
		if (regex_search(line, r)) { return true; }
	}
	return false;	
}


void set_default_index_filename(string &index_filename) {
#if defined(__FreeBSD__)
	// get kern.ostype sysctl to see whether we're running
	// on FreeBSD. If we're not, don't set a standard index
	// file path
	int mib[2] = { CTL_KERN, KERN_OSTYPE };
	char ostype[8];
	size_t len = sizeof(ostype);
	
	int retval = sysctl(mib, 2, ostype, &len, NULL, 0);
	if (retval != 0 && errno != ENOMEM) {
		perror("sysctl");
		return;
	}
	ostype[len - 1] = '\0';
	
	if (0 != strncmp("FreeBSD", ostype, len)) { return; }
	
	// get kern.osrelease sysctl to see whether we want
	// INDEX, INDEX-5 or INDEX-6 etc.
	mib[1] = KERN_OSRELEASE;
	char osrelease[32];
	len = sizeof(osrelease);
	retval = sysctl(mib, 2, osrelease, &len, NULL, 0);
	if (retval != 0 && errno != ENOMEM) {
		perror("sysctl");
		return;
	}
	osrelease[len - 1] = '\0';
	
	// get major version
	char* s = strchr(osrelease, '.');
	if (s == NULL) { return; }
	size_t major_version_str_len = (size_t)(s - osrelease) + 1;
	if (major_version_str_len < 1 or major_version_str_len > 3) { return; }
	char major_version_str[major_version_str_len];
	strlcpy(major_version_str, osrelease, major_version_str_len);
	
	index_filename = "/usr/ports/INDEX-";
	index_filename.append(major_version_str);
#else
	index_filename.clear();
#endif
}
