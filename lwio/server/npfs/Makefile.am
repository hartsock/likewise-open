if ENABLE_STATIC_DRIVERS
noinst_LTLIBRARIES = libnpfs.sys.la
DRIVER_CFLAGS =
DRIVER_LDFLAGS =
else
lib_LTLIBRARIES = libnpfs.sys.la
DRIVER_CFLAGS = -shared
DRIVER_LDFLAGS = -module -shared -avoid-version
endif

libnpfs_sys_la_SOURCES = \
    close.c  \
    create.c \
    createnp.c \
    connectnp.c \
    ccb.c    \
    fcb.c    \
    file_basic_info.c \
    file_access_info.c \
    file_network_open_info.c \
    file_pipe_info.c \
    file_pipe_local_info.c \
    file_std_info.c \
    deviceio.c \
    driver.c \
    fsctl.c \
    globals.c \
    queryinfo.c \
    read.c \
    setinfo.c \
    memory.c \
    pipe.c \
    mdl.c \
    write.c

libnpfs_sys_la_CPPFLAGS = \
    -I@top_srcdir@         \
    -I@top_srcdir@/include \
    -I@top_srcdir@/server/include

libnpfs_sys_la_CFLAGS = $(AM_CFLAGS) $(DRIVER_CFLAGS)

libnpfs_sys_la_LIBADD = \
    @top_builddir@/common/liblwiocommon.la \
    @UUID_LIBS@ \
    @PTHREAD_LIBS@

libnpfs_sys_la_LDFLAGS = $(DRIVER_LDFLAGS)

