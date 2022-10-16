# -----------------------------------------------------------------------------
# first entry
ifeq ($(MODEM),)
MODEM_TYPES	:= demo-adsl demo-vdsl speedtouch ar7 bc63 conexant vinax vigor
BUILD_MODEMS	:= $(addprefix build-,$(MODEM_TYPES))
INSTALL_MODEMS	:= $(addprefix install-,$(MODEM_TYPES))
CLEAN_MODEMS	:= $(addprefix clean-,$(MODEM_TYPES))

# -----------------------------------------------------------------------------
# common targets
.PHONY: all build clean rebuild install doc 
all: build
rebuild: clean all
build: $(BUILD_MODEMS)
install: $(INSTALL_MODEMS)
clean: $(CLEAN_MODEMS)
doc:
	doxygen dsltool.doxygen

# -----------------------------------------------------------------------------
# generic targets
.PHONY: $(BUILD_MODEMS) $(CLEAN_MODEMS) $(INSTALL_MODEMS)
$(BUILD_MODEMS):
	$(MAKE) CC=$(CC) LD=$(LD) CFLAGS="$(CFLAGS)" TARGET=dsltool MODEM=$(@:build-%=%) build
	$(MAKE) CC=$(CC) LD=$(LD) CFLAGS="$(CFLAGS)" TARGET=dsltoold MODEM=$(@:build-%=%) build
$(CLEAN_MODEMS):
	$(MAKE) CC=$(CC) LD=$(LD) CFLAGS="$(CFLAGS)" TARGET=dsltool MODEM=$(@:clean-%=%) clean
	$(MAKE) CC=$(CC) LD=$(LD) CFLAGS="$(CFLAGS)" TARGET=dsltoold MODEM=$(@:clean-%=%) clean

$(INSTALL_MODEMS):
	$(MAKE) INSTALL=$(INSTALL) TARGET_DIR="$(TARGET_DIR)" TARGET=dsltool MODEM=$(@:install-%=%) install
	$(MAKE) INSTALL=$(INSTALL) TARGET_DIR="$(TARGET_DIR)" TARGET=dsltoold MODEM=$(@:install-%=%) install
	
else # $(MODEM)
# -----------------------------------------------------------------------------
# flags and libraries
LDLIBS		+= -lm

EXTRA_CFLAGS	:= -DCFG_$(shell echo $(MODEM) | sed -e 's/\(.*\)/\U\1/' -e 'y/-/_/')

VER		:= rel

FILES		:= $(TARGET)
FILES		+= log
FILES		+= parser
FILES		+= bandplan
FILES		+= parser-$(MODEM)
ifeq ($(TARGET),dsltool)
FILES		+= telnet 
FILES		+= output
FILES		+= graphic
EXTRA_CFLAGS	+= $(shell pkg-config --cflags pangocairo)
EXTRA_CFLAGS	+= -DCFG_COMMAND
LDLIBS		+= $(shell pkg-config --libs pangocairo)
else
ifeq ($(TARGET),dsltoold)
FILES		+= telnet 
FILES		+= collect
EXTRA_CFLAGS	+= -DCFG_DAEMON
LDLIBS		+= -lrt -lcollectdclient
endif # dsltoold
endif # dsltool

HEADER		:= telnet.h
HEADER		+= parser.h
HEADER		+= bandplan.h
HEADER		+= output.h
HEADER		+= graphic.h
HEADER		+= collect.h
HEADER		+= log.h

OBJDIR		:= ./$(TARGET)-$(MODEM).$(VER)
OBJS		:= $(addprefix $(OBJDIR)/, $(addsuffix .o,$(FILES))) 

# -----------------------------------------------------------------------------
# common targets
build: $(OBJDIR) $(TARGET)-$(MODEM)
clean:
	$(VERBOSE)$(RM) -r -f $(OBJDIR)
	$(VERBOSE)$(RM) -f $(TARGET)-$(MODEM)

install:
	$(INSTALL) -D $(TARGET)-$(MODEM) $(TARGET_DIR)/$(TARGET)-$(MODEM)

# -----------------------------------------------------------------------------
# create directories
$(OBJDIR):
	mkdir $@
	
# -----------------------------------------------------------------------------
# link
$(TARGET)-$(MODEM): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# -----------------------------------------------------------------------------
# compile
$(OBJDIR)/%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<

endif # $(MODEM)
# _oOo_
