# 
#        #=======================================================#     _\|/_
#       #  __  _____           ___                    ___       #       /|\
#      #  /      |      ^     |   \  |         ^     |   \     #           _\|/_
#     #   \__    |     / \    |___/  |        / \    |___/    #             /|\
#    #       \   |    /___\   |  \   |       /___\   |   \   #  _\|/_
#   #     ___/   |   /     \  |   \  |____  /     \  |___/  #    /|\
#  #                                                       #             _\|/_
# #=======================================================#               /|\
#
##
##	src/node/dyn/hdyn/xutil/Makefile.am
##..............................................................................
##	Dec 1994		Piet Hut & Steve McMillan
##	Jul 2004		Ernest Mamikonyan
##..............................................................................
##

pkglib_LTLIBRARIES = libxhdyn.la
bin_PROGRAMS = snap_to_image

if !NO_X
    bin_PROGRAMS += hxstarplot
endif

# Note that Qt_pp3 contains code to check for Qt, so OK to include
# it in the library without testing X (NO_X ==> no Qt).

libxhdyn_la_LDFLAGS = ../libhdyn.la
libxhdyn_la_SOURCES = Qt_pp3.C

if HAVE_LIBQT
  bin_PROGRAMS += Qt_pp3
  libxhdyn_la_LDFLAGS += @QT_LIBS@ $(X_PRE_LIBS) -lX11 $(X_EXTRA_LIBS)
  libxhdyn_la_SOURCES += pp3_widget.C moc_pp3_widget.C
  Qt_pp3_CPPFLAGS = -DTOOLBOX
  Qt_pp3_LDADD = libxhdyn.la
  Qt_pp3_SOURCES = Qt_pp3.C
  BUILT_SOURCES = moc_pp3_widget.C
  CLEANFILES = moc_pp3_widget.C
endif

snap_to_image_LDADD = ../libhdyn.la ../../../../gfx/libgfx.la
hxstarplot_LDADD = ../libhdyn.la ../../../../gfx/libgfx.la

snap_to_image_SOURCES = snap_to_image.C
hxstarplot_SOURCES = hxstarplot.C gfx_util.C dyn_util.C

.PHONY: libs
libs: libxhdyn.la

# The "touch" below is to ensure that the .C file exists even if moc
# is not avaliable -- shouldn't be necessary... (Steve, 7/08).
moc_pp3_widget.C: ../../../../../include/pp3_widget.h
	@MOC@ -o $@ $< ; touch $@
