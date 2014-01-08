############################################################################
# Makefile for the Volume Processing Library (libvp)
# Oliver Hinds <oph@bu.edu> 2005-06-02
#######################################################################

# project name
export PROJECT = libvp
export MAJOR_VER = 1
export MINOR_VER = 1
export RELEASE_VER = 1

# operating system, mac, linux and win are supported
export OS = linux

# whether to build a static or shared library
export LIB_TYPE = shared

# IMPORTANT!! CHANGE THIS TO BIG IF ON NON-X86 PLATFORMS!!!
export ENDIAN = little

# whether to compile with debug, optimize flags
export DEBUG = 0
export OPTIM = 1
export STRIP = 1
export PROF = 0
export MEMLEAK = 0

# directories
export LIB_DEST_DIR = /usr/local/lib/
export HDR_DEST_DIR = /usr/local/include/
export LIB_DIR = /tmp/
export SRC_DIR = $(PWD)/src/
export OBJ_DIR = $(PWD)/obj/

################################ APPS ################################

export RM = /bin/rm -v
export ECHO = /bin/echo
export CC = /usr/bin/gcc
export AR = /usr/bin/ar
export INSTALL=sudo /usr/bin/install
export ROOTLN=sudo ln
export LDCONFIG=/sbin/ldconfig

# endian define flag
ifeq ($(ENDIAN),little)
	ENDIAN_FLAG = -DLITTLE_ENDIAN
else
	ENDIAN_FLAG = -DBIG_ENDIAN
endif

# jpeg includes and libs
ifeq ($(OS),mac)

#	FINK_ROOT = /
	LIB_TYPE = static
	JPEG_INCS = -I$(FINK_ROOT)/sw/include
	JPEG_LIBS = -L$(FINK_ROOT)/sw/lib -ljpeg
else
	JPEG_INCS = -I/usr/include
	JPEG_LIBS = -L/usr/lib -ljpeg
endif

# debug flag
ifeq ($(DEBUG),1)
	DEBUG_FLAG = -g
endif

# profile flag
ifeq ($(PROF),1)
	PROF_FLAG = -pg
endif

# optimize flag
ifeq ($(OPTIM),1)
	OPTIM_FLAG = -O
endif

# strip flag
ifeq ($(STRIP),1)
	STRIP_FLAG = -s
endif

# memleak catch flag
ifeq ($(MEMLEAK),1)
	MEMLEAK_FLAG = -DMEMLEAK
endif

# flags for the compiler and linker
ifeq ($(OS),linux)
	export fPIC = -fPIC
endif

export CINCL =  -I$(SRC_DIR) -I$(SRC_DIR)/ljpg -I$(SRC_DIR)/dicom
export CFLAGS = $(fPIC) -Werror -Wall -Wno-unused-result $(MEMLEAK_FLAG) $(PROF_FLAG) $(JPEG_INCS) $(DEBUG_FLAG) $(OPTIM_FLAG) $(STRIP_FLAG) $(CINCL) $(ENDIAN_FLAG)
export LDFLAGS = $(PROF_FLAG) $(JPEG_LIBS)

# differences between mac and *nix
ifeq ($(OS),mac)
	CFLAGS += -DMAC
	LDFLAGS += -lobjc
endif

# specific flags for windows, these override earlier defs of cflags and ldflags
ifeq ($(OS),win)
	LIB_TYPE = static
#        export CFLAGS = -Wall -I/usr/include $(PROF_FLAG) $(JPEG_INCS) $(DEBUG_FLAG) $(OPTIM_FLAG) $(STRIP_FLAG)
	export LDFLAGS = -L/usr/X11R6/lib $(JPEG_LIBS) -lm -lgsl -lz
endif

############################## SUFFIXES ##############################

## if any command to generate target flibvps, the target is deleted.
# (see http://www.gnu.org/manual/make/html_chapter/make_5.html#SEC48)
.DELETE_ON_ERROR:

.SUFFIXES:
.SUFFIXES:  .o .c

# suffix rule for subsidiary source files
# (see http://www.gnu.org/manual/make/html_chapter/make_10.html#SEC111)
$(OBJ_DIR)/%.o: %.c %.h
	@$(ECHO) '[make: building $@]'
	$(CC) $(CFLAGS) -o $@ -c $<

HDR_FILES = $(wildcard *.h)
SRC_FILES = $(wildcard ./*.c)
TMP_FILES = $(patsubst ./%,$(OBJ_DIR)/%,$(SRC_FILES))
OBJ_FILES = $(TMP_FILES:.c=.o)

default: $(PROJECT)
debug:
	$(MAKE) DEBUG=1 OPTIM=0 STRIP=0 $(PROJECT)
$(PROJECT): $(OBJ_FILES)
	@$(ECHO) 'make: building lib$@ for $(OS)...'
	cd $(SRC_DIR) && $(MAKE)
ifeq ($(LIB_TYPE),shared)
	$(CC)  -shared  -Wl,-soname,$(PROJECT).so.$(MAJOR_VER) $(OBJ_DIR)/*.o -o $(LIB_DIR)/$(PROJECT).so.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER) $(LDFLAGS)
	@$(ECHO) '############################################'
	@$(ECHO) 'make: built [$@.so.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER)] successfully!'
	@$(ECHO) '############################################'
else
	$(AR) rcs $(LIB_DIR)/$(PROJECT).a.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER) $(OBJ_DIR)/*.o
	@$(ECHO) '############################################'
	@$(ECHO) 'make: built [$@.a.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER)] successfully!'
	@$(ECHO) '############################################'
endif

############################### INSTALL ################################

install: $(PROJECT)
	$(INSTALL) -m 644 $(SRC_DIR)/*.h $(HDR_DEST_DIR)
	$(INSTALL) -m 644 $(SRC_DIR)/dicom/*.h $(HDR_DEST_DIR)
	$(INSTALL) -m 644 $(SRC_DIR)/ljpg/*.h $(HDR_DEST_DIR)
ifeq ($(LIB_TYPE),shared)
	@$(ECHO) 'make: installing $(PROJECT).so.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER) for $(OS)...'
	$(INSTALL) -m 644 $(LIB_DIR)/$(PROJECT).so.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER) $(LIB_DEST_DIR)
	$(ROOTLN) -sf $(LIB_DEST_DIR)/$(PROJECT).so.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER) $(LIB_DEST_DIR)/$(PROJECT).so.$(MAJOR_VER)
	$(ROOTLN) -sf $(LIB_DEST_DIR)/$(PROJECT).so.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER) $(LIB_DEST_DIR)/$(PROJECT).so
ifeq ($(OS),linux)
	$(LDCONFIG) -n $(LIB_DEST_DIR)
endif
	@$(ECHO) '############################################'
	@$(ECHO) 'make: installed [$(PROJECT).so.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER)] successfully!'
	@$(ECHO) '############################################'
else
	@$(ECHO) 'make: installing $(PROJECT).a.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER) for $(OS)...'
	cp $(LIB_DIR)/$(PROJECT).a.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER) $(LIB_DEST_DIR)
	$(ROOTLN) -sf $(LIB_DEST_DIR)/$(PROJECT).a.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER) $(LIB_DEST_DIR)/$(PROJECT).a.$(MAJOR_VER)
	$(ROOTLN) -sf $(LIB_DEST_DIR)/$(PROJECT).a.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER) $(LIB_DEST_DIR)/$(PROJECT).a
	@$(ECHO) '############################################'
	@$(ECHO) 'make: installed [$(PROJECT).a.$(MAJOR_VER).$(MINOR_VER).$(RELEASE_VER)] successfully!'
	@$(ECHO) '############################################'

endif
############################### CLEAN ################################

clean:
	@$(ECHO) 'make: removing object and autosave files'
	-$(RM) -f $(OBJ_DIR)/*.o *~ $(SRC_DIR)/*~
	cd $(SRC_DIR) && $(MAKE) clean


######################################################################
### $Source: /home/cvs/PROJECTS/VolumeProcessingLibrary/makefile,v $
### Local Variables:
### mode: makefile
### fill-column: 76
### comment-column: 0
### End:
