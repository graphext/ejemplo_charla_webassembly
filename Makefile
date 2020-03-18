
EXEC = example
BINDIR = bin
IDLDIR = idl
OBJDIR = obj
SRCDIR = src
INCLUDEDIR = include
LIBDIR = lib
DEBUG = 0

EMCC_WASM_BACKEND=1
TOTAL_MEMORY=16MB
TOTAL_STACK=1MB
MEMORY_GROWTH_STEP=-1

ifeq ($(DEBUG), 1)
	DEBUG_FLAGS = -g4 -O1 --memoryprofiler -s ASSERTIONS=2 -s ALIASING_FUNCTION_POINTERS=0 \
		-s SAFE_HEAP=1 -s DEMANGLE_SUPPORT=1 --source-map-base http://localhost:8082/ # -fsanitize=undefined
else
	DEBUG_FLAGS = -O3 --llvm-lto 3 -s ASSERTIONS=0 \
		-DGLOBAL_LOG_LEVEL=INFO
endif

ENV_VARS = EMCC_WASM_BACKEND=$(EMCC_WASM_BACKEND)
CC = emcc
CXX = em++
AR = emar
COMMON_FLAGS = -Werror -Wall $(DEBUG_FLAGS) -fdiagnostics-color=auto \
	 -Wcast-align -Wover-aligned -s WARN_UNALIGNED=1 \
	 -s WASM=1 -s STRICT=1 -s ALLOW_MEMORY_GROWTH=1 \
	 -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s NO_FILESYSTEM=1 -fno-rtti \
	 -s TOTAL_MEMORY=$(TOTAL_MEMORY) -s TOTAL_STACK=$(TOTAL_STACK)
CFLAGS = $(COMMON_FLAGS) -I$(INCLUDEDIR) -I$(IDLDIR) -std=c17
CPPFLAGS = $(COMMON_FLAGS) -I$(INCLUDEDIR) -I$(IDLDIR) -std=c++17
LD = $(CXX)
LDFLAGS = $(COMMON_FLAGS) -L$(LIBDIR) --post-js $(IDLDIR)/glue.js -llzip

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
	IDL_CHECKS="ALL#DEFAULT" python "${EM_DIR}"/tools/webidl_binder.py $< $(IDLDIR)/$*; rm parser.out WebIDLGrammar.pkl

$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	$(ENV_VARS) $(CC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o:	$(SRCDIR)/%.cpp
	$(ENV_VARS) $(CXX) -c $(CPPFLAGS) $< -o $@

$(LIBDIR)/liblzip.a:
	cd $(INCLUDEDIR)/lzip/; $(ENV_VARS) make CC="$(CC)" AR="$(AR)" CFLAGS="$(COMMON_FLAGS)"


clean:
	rm -rf $(LIBDIR)/* $(OBJDIR)/* $(IDLDIR)/*.cpp $(IDLDIR)/*.js $(BINDIR)/$(EXEC).js* $(BINDIR)/$(EXEC).wasm* $(BINDIR)/$(EXEC).wast
