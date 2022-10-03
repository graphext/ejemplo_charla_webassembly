ifeq (, $(shell which python3))
	PYTHON = python
else
	PYTHON = python3
endif

EXEC = example
BINDIR = bin
IDLDIR = idl
OBJDIR = obj
SRCDIR = src
INCLUDEDIR = include
LIBDIR = lib
DEBUG = 0

INITIAL_MEMORY=16MB
MAXIMUM_MEMORY=2GB
TOTAL_STACK=1MB
MEMORY_GROWTH_GEOMETRIC_STEP=0.20
MEMORY_GROWTH_GEOMETRIC_CAP=96MB

ifeq ($(DEBUG), 1)
# SAFE_HEAP=1 produces stack overflow
	DEBUG_COMP_FLAGS = -gsource-map -O2 --memoryprofiler # -fsanitize=undefined
	DEBUG_LD_FLAGS = -sASSERTIONS=2 -sSTACK_OVERFLOW_CHECK=2 \
		-Wno-limited-postlink-optimizations \
		-sSAFE_HEAP=1 -sDEMANGLE_SUPPORT=1 --source-map-base http://localhost:8082/
else
	DEBUG_COMP_FLAGS = -O3 -flto=full -fwhole-program-vtables -fvisibility=hidden
	DEBUG_LD_FLAGS = -sASSERTIONS=0 --lto-whole-program-visibility
endif

CC = emcc
CXX = em++
LD = $(CXX)
AR = emar
COMMON_FLAGS = -Werror -Wall $(DEBUG_COMP_FLAGS) -fdiagnostics-color=auto \
	 -Wcast-align -Wover-aligned -Wno-legacy-settings \
	 -Wno-dollar-in-identifier-extension \
	 -sSTRICT=1 -DEMSCRIPTEN=1 \
	 -fexceptions -sDISABLE_EXCEPTION_CATCHING=1 \
	 -fno-rtti -fno-strict-aliasing \
	 -DNDEBUG
CFLAGS = $(COMMON_FLAGS) -I$(INCLUDEDIR) -I$(IDLDIR) -std=c18
CPPFLAGS = $(COMMON_FLAGS) -I$(INCLUDEDIR) -I$(IDLDIR) -std=c++20
LDFLAGS = $(COMMON_FLAGS) $(DEBUG_LD_FLAGS) -L$(LIBDIR) --post-js $(IDLDIR)/glue.js \
	-sLLD_REPORT_UNDEFINED -sALLOW_UNIMPLEMENTED_SYSCALLS \
	-sENVIRONMENT=web \
	-sBINARYEN_EXTRA_PASSES=--one-caller-inline-max-function-size=20 \
	-sNO_EXIT_RUNTIME=1 -sNO_FILESYSTEM=1 -sSUPPORT_ERRNO=0 \
	-sTEXTDECODER=2 \
	-sALLOW_MEMORY_GROWTH=1 \
	-sINITIAL_MEMORY=$(INITIAL_MEMORY) -sMAXIMUM_MEMORY=$(MAXIMUM_MEMORY) -sTOTAL_STACK=$(TOTAL_STACK) \
	-sMEMORY_GROWTH_GEOMETRIC_STEP=$(MEMORY_GROWTH_GEOMETRIC_STEP) -sMEMORY_GROWTH_GEOMETRIC_CAP=$(MEMORY_GROWTH_GEOMETRIC_CAP) \
	-sDECLARE_ASM_MODULE_EXPORTS=0 \
	-sEXPORTED_FUNCTIONS=['_main','_malloc','_free'] \
	-llzip

IDLS=$(patsubst $(IDLDIR)/%.idl,$(IDLDIR)/%.cpp,$(wildcard $(IDLDIR)/*.idl))
OBJS=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(shell find $(SRCDIR) -name '*.c'))
OBJS+=$(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(shell find $(SRCDIR) -name '*.cpp'))
OBJS+= $(LIBDIR)/liblzip.a

uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))
OBJDIRS=$(call uniq,$(dir $(OBJS)))

$(BINDIR)/$(EXEC):	$(OBJDIRS) $(IDLS) $(OBJS)
	$(ENV_VARS) $(LD) -o $@.js $(filter-out $(wildcard */**/*.a),$(OBJS)) $(LDFLAGS)
	@echo "Compiled successfully"

$(OBJDIRS):
	mkdir -p $@

$(IDLDIR)/%.cpp: $(IDLDIR)/%.idl
	IDL_CHECKS="DEFAULT" $(PYTHON) "$(EM_DIR)"/tools/webidl_binder.py $< $(IDLDIR)/$*

$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	$(ENV_VARS) $(CC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o:	$(SRCDIR)/%.cpp
	$(ENV_VARS) $(CXX) -c $(CPPFLAGS) $< -o $@

$(LIBDIR)/liblzip.a:
	cd $(INCLUDEDIR)/lzip/; $(ENV_VARS) make CC="$(CC)" AR="$(AR)" CFLAGS="$(COMMON_FLAGS)"


clean:
	rm -rf $(LIBDIR)/* $(OBJDIR)/* $(IDLDIR)/*.cpp $(IDLDIR)/*.js $(BINDIR)/$(EXEC).js* $(BINDIR)/$(EXEC).wasm* $(BINDIR)/$(EXEC).wast
