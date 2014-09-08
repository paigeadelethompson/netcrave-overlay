# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI=5

inherit git-r3

DESCRIPTION="Hayes AT-compatible modem emulator"
HOMEPAGE="https://github.com/nandhp/atduck"
SRC_URI=""
EGIT_REPO_URI="https://github.com/nandhp/atduck.git"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS=""

#SRC_TEST="do"

DEPEND="dev-perl/IO-Tty dev-lang/perl[ithreads]"

#TODO can use SLiRP? 
src_install() { 
	pod2man atduck.pod > atduck.7
	dodir /usr/share/ATduck
	dodir /usr/share/man/man7
	cp -r lib/ ${ED}/usr/share/ATduck
	cp atduck ${ED}/usr/share/ATduck
	cp -r slirp/ ${ED}/usr/share/ATduck
	cp atduck.7 ${ED}/usr/share/man/man7
	#TODO find PERL_HOME, and just install atduck bin to /usr/bin
	#dosym /usr/share/ATduck/atduck /usr/bin/atduck
	#/usr/share/ATduck/lib/ATduck/ /usr/lib64/perl5/vendor_perl/5.18.2/x86_64-linux-thread-multi/ATduck 
}
