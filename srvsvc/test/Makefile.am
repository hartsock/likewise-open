AM_CPPFLAGS = -I$(top_srcdir)/include

noinst_PROGRAMS = testsrvsvc test_srvshare

testsrvsvc_SOURCES = \
    net_connectionenum.c \
    net_fileclose.c      \
    net_fileenum.c       \
    net_filegetinfo.c    \
    net_remotetod.c      \
    net_servergetinfo.c  \
    net_serversetinfo.c  \
    net_sessionenum.c    \
    net_shareadd.c       \
    net_sharedel.c       \
    net_shareenum.c      \
    net_sharegetinfo.c   \
    net_sharesetinfo.c   \
    params.c             \
    test.c               \
    test_srvsvc.c

SRVSVC_INCLUDES= \
    -I$(SRVSVC_INCDIR)    \
    -I$(top_srcdir)       \
    $(DCERPC_INCLUDES)    \
    $(LIBUNISTR_INCLUDES)

testsrvsvc_CFLAGS =       \
    $(AM_CPPFLAGS)        \
    $(SRVSVC_INCLUDES)    \
    -D_GNU_SOURCE

testsrvsvc_LDFLAGS=       \
    -rpath ${libdir}      \
    $(DCERPC_LDFLAGS)     \
    $(LIBUNISTR_LDFLAGS)

testsrvsvc_LDADD =        \
    $(DCERPC_LIBS)        \
    $(LIBUNISTR_LIBS)     \
    $(top_builddir)/client/libsrvsvc.la

test_srvshare_SOURCES = test_srvshare.c

test_srvshare_CFLAGS = \
    $(AM_CPPFLAGS) \
    $(SRVSVC_INCLUDES)

test_srvshare_LDADD = \
	-llwioclient -llwbase -llwbase_nothr \
        -llwadvapi -llwadvapi_nothr -llsaclient