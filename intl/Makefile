subdir = intl
routines = bindtextdom dcgettext dgettext gettext	\
	   finddomain loadmsgcat localealias textdomain
distribute = gettext.h gettextP.h hash-string.h libgettext.h

include ../Rules

CPPFLAGS += -D'GNULOCALEDIR="$(localedir)"' \
	    -D'LOCALE_ALIAS_PATH="$(localedir):$(nlsdir)"'
