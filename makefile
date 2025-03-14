# ----------------------------
# Makefile Options
# ----------------------------

NAME = RTX
ICON = icon.png
DESCRIPTION = "Raymarching"
COMPRESSED = NO
ARCHIVED ?= NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
