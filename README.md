# psearch
psearch is a utility for making browsing the FreeBSD ports easier.
It does this showing you search results quickly and without any fuss.
It is a standalone C++ program without any dependencies.

## Example
    $ psearch python hashing
    devel/py-hash_ring        Implementation of consistent hashing in Python
    misc/py-python-geohash    Fast, accurate python geohashing library
    security/py-passlib       Python password hashing framework supporting over 30 schemes

## Installation
psearch is available in the FreeBSD ports as ports-mgmt/psearch. You can
installing it like any other port or pkg, e.g. by running
`pkg install psearch` or
`cd /usr/ports/ports-mgmt/psearch; make install clean` or
`portmaster ports-mgmt/psearch`.

## Documentation
After installation, the psearch(1) manual page will be available.

## License
psearch is released under the standard BSD 2 clause license.
