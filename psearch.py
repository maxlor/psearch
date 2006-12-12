#!/usr/bin/env python

#############################################################################
#
# psearch, a utility for searching the FreeBSD ports.
#
# Copyright (C) 2006 Benjamin Lutz (http://public.xdi.org/=Benjamin.Lutz)
#
# Redistribution, use (in source and binary form) and modification of this
# script are permitted provided that the following conditions are met:
# 1. Redistributions of this script must contain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY OR CONSEQUENTAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#############################################################################

import optparse, os.path, re, sys

VERSION = 'psearch 1.2'
COPYRIGHT = 'Copyright (C) 2006 Benjamin Lutz (http://public.xdi.org/=Benjamin.Lutz)'

EXIT_SUCCESS = 0
EXIT_PARAM_ERROR = 1
EXIT_FILE_ERROR = 2

INDEX_PATH = '/usr/ports/INDEX'
if sys.platform.startswith('freebsd'):
	major_version = int(sys.platform[7:])
	if major_version >= 5: INDEX_PATH += '-%s' % major_version


def main():
	(options, arguments) = parse_commandline_arguments()
	
	if options.inverse_pattern is None: options.inverse_pattern = tuple()
	if options.match_or: arguments = ('|'.join(arguments), )
	if not os.path.isfile(options.file):
		sys.stderr.write('Error: "%s" does not exist or is not a regular file.\n' % options.file)
		if options.file == INDEX_PATH and sys.platform.startswith('freebsd'):
			sys.stderr.write('\nYou can create the INDEX file by executing the following commands as root:\n\n')
			sys.stderr.write('  cd /usr/ports\n')
			sys.stderr.write('  make fetchindex\n\n')
			sys.stderr.write('If you don\'t have an internet connection, using "make index" instead of\n')
			sys.stderr.write('"make fetchindex" will work too, although it will take a long time.\n')
		sys.exit(EXIT_PARAM_ERROR)
		
	try:
		precompile_pattern = lambda x: re.compile(x, re.IGNORECASE)
		patterns = map(precompile_pattern, arguments)
		inverse_patterns = map(precompile_pattern, options.inverse_pattern)
	except Exception, e:
		sys.stderr.write('Pattern Error: %s\n' % e)
		sys.exit(EXIT_PARAM_ERROR)
	
	search_index(options.file, patterns, inverse_patterns, options)


def parse_commandline_arguments():
	'''Parse command line arguments and return them as (options, arguments) tuple.'''
	
	usage = '%prog [options] PATTERN [PATTERN ...]'
	description = 'Searches ports for PATTERN. PATTERN is a case-insensitive regular expression. ' + \
		'If there is more than one pattern, each of them is searched for. By default, ports are shown that match ' + \
		'all patterns, use -o to show ports that match at least one pattern. ' + \
		'By default, the name and the short description are searched. If you specify the -s option, then the long ' + \
		'description is searched as well.'
	parser = optparse.OptionParser(usage = usage, description = description, version = "%s\n%s" % (VERSION, COPYRIGHT))
	parser.set_defaults(file = INDEX_PATH, long_description = False, match_or = False, search_long = False)
	parser.add_option('-c', '--category', type = 'string', dest = 'category',
		help = 'Only search for ports in CATEGORY. Speeds up searching.')
	parser.add_option('-f', '--file', type = 'string', dest = 'file',
		help = 'Path to INDEX file. Default: %s' % INDEX_PATH)
	parser.add_option('-l', '--long', action = 'store_true', dest = 'long_description',
		help = 'Display long description (pkg-descr file) for any match found.')
	parser.add_option('-n', '--name', action = 'store_true', dest = 'print_name',
		help = 'Print canonical name of a port, including its version.')
	parser.add_option('-o', '--or', action = 'store_true', dest = 'match_or', help = 'Search for ports that match ' +
		'any PATTERN, instead of all of them.')
	parser.add_option('-s', '--search_long', action = 'store_true', dest = 'search_long',
		help = 'Search long descriptions (pkg-descr file). Slows down searching.')
	parser.add_option('-v', '--inverse', action = 'append', dest = 'inverse_pattern', metavar = 'INVERSE_PATTERN',
		help = 'Searches for ports that do not match a pattern. May be specified several times.')
	return parser.parse_args() 


def search_index(index_path, patterns, inverse_patterns, options):
	'''Search INDEX file. Ports that are found are printed to stdout.
	@param index_path: path to INDEX file
	@param patterns: a list of patterns to be matched.
	@param inverse_patterns: a list of patterns to not be matched.
	@param options: an options object as returned by the command line parser.'''
	
	try:
		f = file(index_path)
	except IOError, e:
		sys.stderr.write('Error: %s: %s\n' % (e.strerror, e.filename))
		sys.exit(EXIT_FILE_ERROR)
	
	for line in f:
		if options.category is None:
			(pkgname, path, prefix, description, description_path, _) = line.split('|', 5)
		else:
			(pkgname, path, prefix, description, description_path, maintainer, categories, _) = line.split('|', 7)
			categories = categories.split()
			if options.category not in categories: continue			
		
		path = path[11:] # strip leading '/usr/ports/'
		
		match = True
		for pattern in patterns:
			match = pattern.search(pkgname) is not None or pattern.search(description) is not None
			if options.search_long and not match:
				try:
					descr = ''.join(file(description_path))
					match = pattern.search(descr) is not None
				except IOError:
					pass
			if not match: break
		if not match: continue
		
		inverse_match = False
		for pattern in inverse_patterns:
			inverse_match = pattern.search(pkgname) is not None or pattern.search(description) is not None
			if options.search_long and inverse_match:
				try:
					descr = ''.join(file(description_path))
					inverse_match = pattern.search(descr) is not None
				except IOError:
					pass
			if inverse_match: break
		if inverse_match: continue
			
		print_port(pkgname, path, description, description_path, options)


def print_port(pkgname, path, description, description_path, options):
	'''Format and print a matched port.'''
	
	
	if options.print_name:
		a = "%-24s %s" % (pkgname, path)
		print '%-49s %s' % (a, description)
	else:
		print '%-25s %s' % (path, description)
		
	if options.long_description:
		for line in file(description_path):
			sys.stdout.write('    ' + line)
		print ''


if __name__ == '__main__':
	try:
		import psyco
		psyco.full()
	except:
		pass
	main()
