# Copyright 1999-2016 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Id$

EAPI=6

inherit cmake-utils eutils multilib versionator user

DESCRIPTION="Anope IRC Services"
HOMEPAGE="https://anope.org"
SRC_URI="https://github.com/anope/anope/releases/download/${PV}/${P}-source.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="amd64 x86"
IUSE="mysql sqlite pcre posix gnutls ssl sql tre ldap anope_modules_sql_auth
	anope_modules_sql_log anope_modules_sql_oper anope_modules_ldap_auth
	anope_modules_ldap_oper anope_modules_irc2sql anope_modules_chanstats
	anope_modules_cs_fantasy_stats anope_modules_cs_fantasy_top"

REQUIRED_USE="sql? ( || ( mysql sqlite ) )
	mysql? ( sql )
	sqlite? ( sql )
	anope_modules_sql_auth? ( sql )
	anope_modules_sql_oper? ( sql )
	anope_modules_sql_log? ( sql )
	anope_modules_cs_fantasy_stats? ( sql )
	anope_modules_cs_fantasy_top? ( sql )
	anope_modules_chanstats? ( sql )
	anope_modules_irc2sql? ( sql )
	anope_modules_ldap_auth? ( ldap )
	anope_modules_ldap_oper? ( ldap )"


DEPEND="
	mysql? ( virtual/mysql )
	ssl? ( dev-libs/openssl:= )
	gnutls? ( net-libs/gnutls dev-libs/libgcrypt:0 )
	ldap? ( net-nds/openldap )
	pcre? ( dev-libs/libpcre )
	sqlite? ( >=dev-db/sqlite-3.0 )
	tre? ( dev-libs/tre )"
RDEPEND="${DEPEND}"

pkg_setup() {
	enewgroup anope
	enewuser anope -1 -1 -1 anope
}

src_unpack() {
	unpack ${A}
	mv "${P}-source" "${P}"
}

#void anope_use_extra(const useflag_t useflag, const char* modulefile)
anope_use_extra() {
	local useflag=$1
	local modulefile=$2
	local destfile=$(basename $2)
	if use $useflag
	then
		ln -s "${WORKDIR}/${P}/modules/extra/$modulefile" "${WORKDIR}/${P}/modules/$destfile" || die "Failed to use $modulefile"
	fi
}

src_prepare() {
	anope_use_extra mysql   						"m_mysql.cpp"
	anope_use_extra sqlite  						"m_sqlite.cpp"
	anope_use_extra ssl 							"m_ssl_openssl.cpp"
	anope_use_extra gnutls  						"m_ssl_gnutls.cpp"
	anope_use_extra posix   						"m_regex_posix.cpp"
	anope_use_extra pcre							"m_regex_pcre.cpp"
	anope_use_extra tre 							"m_regex_tre.cpp"
	anope_use_extra ldap							"m_ldap.cpp"
	anope_use_extra anope_modules_sql_auth  		"m_sql_authentication.cpp"
	anope_use_extra anope_modules_sql_log   		"m_sql_log.cpp"
	anope_use_extra anope_modules_sql_oper  		"m_sql_oper.cpp"
	anope_use_extra anope_modules_ldap_auth 		"m_ldap_authentication.cpp"
	anope_use_extra anope_modules_ldap_oper 		"m_ldap_oper.cpp"
	anope_use_extra anope_modules_chanstats 		"stats/m_chanstats.cpp"
	anope_use_extra anope_modules_cs_fantasy_top	"stats/cs_fantasy_top.cpp"
	anope_use_extra anope_modules_cs_fantasy_stats  "stats/cs_fantasy_stats.cpp"
	anope_use_extra anope_modules_irc2sql   		"stats/irc2sql/irc2sql.cpp"
	anope_use_extra anope_modules_irc2sql   		"stats/irc2sql/irc2sql.h" 
	anope_use_extra anope_modules_irc2sql   		"stats/irc2sql/tables.cpp"
	anope_use_extra anope_modules_irc2sql   		"stats/irc2sql/utils.cpp"

	unset -f anope_use_extra

	eapply_user
}

src_configure() {
	local mycmakeargs=(
		-DCMAKE_INSTALL_PREFIX="${ROOT}usr/lib/anope/"
		-DCONF_DIR="/etc/anope/"
		-DLOGS_DIR="/var/log/anope/"
	)
	cmake-utils_src_configure
}

src_install() {
	cmake-utils_src_install
	newinitd "${FILESDIR}/anope-init" "${PN}"
	mkdir -p "${D}usr/bin"
	mkdir -p "${D}var/log/anope"
	ln -s "${D}etc/anope" "${D}usr/lib/anope/conf"
	ln -s "${D}usr/lib/anope/bin/services" "${D}usr/bin/services"
	ln -s "${D}usr/lib/anope/bin/anopesmtp" "${D}usr/bin/anopesmtp"
	fowners -R "anope:anope" "/etc/anope" "/usr/lib/anope" "/var/log/anope"
	fperms -R 0500 "/etc/anope" "/usr/lib/anope/lib" "/usr/lib/anope/bin"
	fperms -R 0755 "/var/log/anope"
	fperms -R 0555 "/usr/lib/anope/locale"
	fperms -R 0750 "/usr/lib/anope/data"
}
