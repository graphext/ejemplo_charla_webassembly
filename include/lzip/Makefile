
LIBNAME = liblzip
SRCDIR = .
INCLUDEDIR = .
LIBDIR = ../../lib
OBJDIR = .

CC ?= gcc
AR ?= ar
override CFLAGS := $(CFLAGS) -Wall -I$(INCLUDEDIR) -O3

OBJS=lzlib.o

$(LIBDIR)/$(LIBNAME).a:	clean $(OBJS)
	$(AR) crvs $(LIBDIR)/$(LIBNAME).a *.o
	rm -f $(OBJDIR)/*.o

$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJDIR)/*.o
