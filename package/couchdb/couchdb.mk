################################################################################
#
# CouchDB
#
################################################################################

COUCHDB_VERSION = 1.7.2
COUCHDB_SITE = \
	http://archive.apache.org/dist/couchdb/source/$(COUCHDB_VERSION)
COUCHDB_SOURCE = apache-couchdb-$(COUCHDB_VERSION).tar.gz
COUCHDB_LICENSE = Apache-2.0
COUCHDB_LICENSE_FILES = LICENSE
COUCHDB_DEPENDENCIES = \
	host-pkgconf \
	erlang \
	icu \
	libcurl \
	openssl \
	spidermonkey185

COUCHDB_CONF_ENV = \
	CURL_CONFIG=$(STAGING_DIR)/usr/bin/curl-config \
	ICU_CONFIG=$(STAGING_DIR)/usr/bin/icu-config
COUCHDB_CONF_OPTS = $(if $(BR2_INIT_SYSV)$(BR2_INIT_BUSYBOX),,--disable-init)

ifeq ($(BR2_PACKAGE_LIBXCRYPT),y)
COUCHDB_DEPENDENCIES += libxcrypt
endif

# CouchDB's build system mixes the erl for the host and the erl for the
# target.
define COUCHDB_FIX_ERL_PATH
	sed -i -re 's,$(HOST_DIR),\/usr,' \
		$(TARGET_DIR)/usr/bin/couchdb \
		$(TARGET_DIR)/usr/bin/couch-config
endef
COUCHDB_POST_INSTALL_TARGET_HOOKS += COUCHDB_FIX_ERL_PATH

define COUCHDB_INSTALL_INIT_SYSV
	mv $(TARGET_DIR)/etc/init.d/couchdb $(TARGET_DIR)/etc/init.d/S97couchdb
	install -m 755 package/couchdb/S96prepare-couchdb \
		$(TARGET_DIR)/etc/init.d/
endef

define COUCHDB_USERS
	couchdb -1 couchdb -1 ! - /bin/sh - CouchDB Server
endef

define COUCHDB_PERMISSIONS
	/etc/couchdb		r	755	couchdb	couchdb - - - -
	/var/lib/couchdb	d	750	couchdb	couchdb - - - -
endef

$(eval $(autotools-package))
