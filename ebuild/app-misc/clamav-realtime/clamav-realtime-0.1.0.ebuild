# Copyright 1999-2015 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: https://raw.githubusercontent.com/jaypeche/Clamav-realtime/master/ebuild/app-misc/clamav-realtime/clamav-realtime-0.1.0.ebuild


EAPI="4"

inherit eutils linux-info

DESCRIPTION="clamav-realtime: ClamAV realtime scanner"
HOMEPAGE="http://wiki.pingwho.org"
SRC_URI="http://www.pingwho.org/pub/gentoo/ftp/overlay/distfiles/${P}.tar.xz"

RESTRICT="nomirror"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="+X +gtk"

DEPEND="sys-libs/glibc
	x11-libs/gtk+
        >=x11-libs/libnotify-0.7.5-r1
        >=app-antivirus/clamav-0.97.8-r2
        >=sys-fs/inotify-tools-3.13-r1"
RDEPEND="${DEPEND}"

src_unpack() {
    unpack ${A}
    cd "${S}"
    epatch "${FILESDIR}/${PN}-gentoo.patch" || die "epatch failed !"
}

pkg_setup() {
	if ! linux_config_exists; then
		eerror "Kernel configuration file doesn't exist."
	elif ! linux_chkconfig_present INOTIFY_USER; then
		eerror "WARNING : CONFIG_INOTIFY_USER not enabled in kernel."
	fi
}

src_compile() {
    if [ -f Makefile ] || [ -f GNUmakefile ] || [ -f makefile ]; then
        emake || die "emake failed"
    fi
}

src_install() {
    emake DESTDIR="${D}" install || die "Install failed"
}

pkg_postinst() {
    einfo
    einfo "NOTE: clamav-realtime was installed in /usr/local/bin"
    einfo "      For security, and for a better graphical integration,"
    einfo "      it must be run as user !"
    einfo
}
