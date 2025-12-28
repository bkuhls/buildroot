################################################################################
#
# rtptools
#
################################################################################

RTPTOOLS_VERSION = 246-g520c0a3722f37cd243d6908063d86a0306ace088
RTPTOOLS_SITE = $(call github,irtlab,rtptools,$(RTPTOOLS_VERSION))
RTPTOOLS_LICENSE = BSD-3-Clause
RTPTOOLS_LICENSE_FILES = LICENSE

define RTPTOOLS_CONFIGURE_LOCAL
	( \
	echo 'PREFIX="$(TARGET_DIR)"'; \
	echo 'HAVE_BIGENDIAN=$(if $(filter "BIG",$(BR2_ENDIAN)),1,0)'; \
	echo 'HAVE_ERR=1'; \
	echo 'HAVE_GETOPT=1'; \
	echo 'HAVE_GETTIMEOFDAY=1'; \
	echo 'HAVE_MSGCONTROL=1'; \
	echo 'HAVE_LNSL=0'; \
	echo 'HAVE_LSOCKET=0'; \
	echo 'HAVE_PROGNAME=1'; \
	echo 'HAVE_STRTONUM=1'; \
	echo 'HAVE_WINDOWS=0'; \
	) > $(@D)/configure.local
endef
RTPTOOLS_PRE_CONFIGURE_HOOKS += RTPTOOLS_CONFIGURE_LOCAL

$(eval $(autotools-package))
