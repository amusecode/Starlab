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
##	src/Makefile.am
##..............................................................................
##	Dec 1994		Piet Hut & Steve McMillan
##	Jun 2004		Ernest Mamikonyan
##..............................................................................
##

SUBDIRS = std node star hydro harpsims

if !NO_X
    SUBDIRS += gfx
endif

# Comment out these lines to avoid exporting the autools packages.

EXTRA_DIST = packages/autoconf-2.59.tar.gz \
	     packages/automake-1.8.5.tar.gz \
	     packages/libtool-1.5.6.tar.gz
