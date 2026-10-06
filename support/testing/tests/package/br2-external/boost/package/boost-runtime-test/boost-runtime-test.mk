################################################################################
#
# boost-runtime-test
#
################################################################################

BOOST_RUNTIME_TEST_SITE = $(BOOST_RUNTIME_TEST_PKGDIR)
BOOST_RUNTIME_TEST_SITE_METHOD = local
BOOST_RUNTIME_TEST_DEPENDENCIES = boost

BOOST_RUNTIME_TEST_ALL_LIBS = \
	atomic chrono container context contract coroutine date_time \
	exception fiber filesystem graph iostreams json locale log math \
	nowide program_options random regex serialization stacktrace \
	system test thread timer type_erasure url wave

BOOST_RUNTIME_TEST_LIBS = \
	$(foreach lib,$(BOOST_RUNTIME_TEST_ALL_LIBS), \
		$(if $(BR2_PACKAGE_BOOST_$(call UPPERCASE,$(lib))),$(lib)))

# Link flags come from the cmake config files installed by boost, the
# upstream tests are built from the boost source tree
BOOST_RUNTIME_TEST_CONF_OPTS = \
	-DBOOST_SOURCE_DIR=$(BOOST_DIR) \
	-DBOOST_RUNTIME_TEST_LIBS="$(subst $(space),;,$(strip $(BOOST_RUNTIME_TEST_LIBS)))"

ifeq ($(BR2_PACKAGE_BOOST_PYTHON),y)
BOOST_RUNTIME_TEST_DEPENDENCIES += python3
BOOST_RUNTIME_TEST_CONF_OPTS += \
	-DBOOST_RUNTIME_TEST_PYTHON_INCLUDE_DIR=$(STAGING_DIR)/usr/include/python$(PYTHON3_VERSION_MAJOR) \
	-DBOOST_RUNTIME_TEST_PYTHON_SITE_DIR=lib/python$(PYTHON3_VERSION_MAJOR)/site-packages
endif

$(eval $(cmake-package))
