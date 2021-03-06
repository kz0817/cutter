#!/bin/bash

BASE_DIR="`dirname $0`"
export BASE_DIR
top_dir="$BASE_DIR/.."

if test x"$RUNNING_TEST" = x"yes"; then
    exit 0
fi

if test -z "$MAKE"; then
    if which gmake 2>&1 > /dev/null; then
	MAKE=gmake
    else
	MAKE=make
    fi
fi

if test x"$NO_MAKE" != x"yes"; then
    RUNNING_TEST=yes
    export RUNNING_TEST
    $MAKE -C $top_dir/ check INTLTOOL_UPDATE=: > /dev/null || exit 1
fi

if test -z "$CUTTER"; then
    CUTTER="$BASE_DIR/$($MAKE -s -C $BASE_DIR echo-cutter)"
fi

CUTTER_ARGS=
CUTTER_WRAPPER=
if test x"$CUTTER_DEBUG" = x"yes"; then
    CUTTER_WRAPPER="$top_dir/libtool --mode=execute gdb --args"
    CUTTER_ARGS="--keep-opening-modules"
elif test x"$CUTTER_CHECK_LEAK" = x"yes"; then
    CUTTER_WRAPPER="$top_dir/libtool --mode=execute valgrind "
    CUTTER_WRAPPER="$CUTTER_WRAPPER --leak-check=full --show-reachable=yes -v"
    CUTTER_ARGS="--keep-opening-modules"
fi

CUT_ICONS_DIR=$(cd $top_dir/data/icons; pwd)
CUT_UI_MODULE_DIR=$top_dir/module/ui/.libs
CUT_UI_FACTORY_MODULE_DIR=$top_dir/module/ui/.libs
CUT_UI_DATA_DIR=$(cd $top_dir/module/ui; pwd)
CUT_REPORT_MODULE_DIR=$top_dir/module/report/.libs
CUT_REPORT_FACTORY_MODULE_DIR=$top_dir/module/report/.libs
CUT_STREAM_MODULE_DIR=$top_dir/module/stream/.libs
CUT_STREAM_FACTORY_MODULE_DIR=$top_dir/module/stream/.libs
CUT_LOADER_CUSTOMIZER_MODULE_DIR=$top_dir/module/loader-customizer/.libs
CUT_LOADER_CUSTOMIZER_FACTORY_MODULE_DIR=$top_dir/module/loader-customizer/.libs

export CUTTER
export CUT_ICONS_DIR
export CUT_UI_MODULE_DIR
export CUT_UI_FACTORY_MODULE_DIR
export CUT_UI_DATA_DIR
export CUT_REPORT_MODULE_DIR
export CUT_REPORT_FACTORY_MODULE_DIR
export CUT_STREAM_MODULE_DIR
export CUT_STREAM_FACTORY_MODULE_DIR
export CUT_LOADER_CUSTOMIZER_MODULE_DIR
export CUT_LOADER_CUSTOMIZER_FACTORY_MODULE_DIR

CUTTER_ARGS="$CUTTER_ARGS -s $BASE_DIR --exclude-directory fixtures"
CUTTER_ARGS="$CUTTER_ARGS --exclude-directory lib"
if echo "$@" | grep -- --mode=analyze > /dev/null; then
    :
else
    CUTTER_ARGS="$CUTTER_ARGS --stream=xml --stream-directory $top_dir/log"
fi
if test x"$USE_GTK" = x"yes"; then
    CUTTER_ARGS="-u gtk $CUTTER_ARGS"
fi

case `uname` in
    CYGWIN*)
	PATH="$top_dir/cutter/.libs:$PATH"
	PATH="$top_dir/gcutter/.libs:$PATH"
	PATH="$top_dir/cppcutter/.libs:$PATH"
	PATH="$top_dir/gdkcutter-pixbuf/.libs:$PATH"
	PATH="$top_dir/soupcutter/.libs:$PATH"
	PATH="$top_dir/test/lib/.libs:$PATH"
	;;
    Darwin)
	DYLD_LIBRARY_PATH="$top_dir/cppcutter/.libs:$DYLD_LIBRARY_PATH"
	DYLD_LIBRARY_PATH="$top_dir/gdkcutter-pixbuf/.libs:$DYLD_LIBRARY_PATH"
	DYLD_LIBRARY_PATH="$top_dir/soupcutter/.libs:$DYLD_LIBRARY_PATH"
	export DYLD_LIBRARY_PATH
	;;
    *BSD)
	LD_LIBRARY_PATH="$top_dir/cppcutter/.libs:$LD_LIBRARY_PATH"
	LD_LIBRARY_PATH="$top_dir/gdkcutter-pixbuf/.libs:$LD_LIBRARY_PATH"
	LD_LIBRARY_PATH="$top_dir/soupcutter/.libs:$LD_LIBRARY_PATH"
	export LD_LIBRARY_PATH
        ;;
    *)
	:
	;;
esac

LC_ALL=C
export LC_ALL
$CUTTER_WRAPPER $CUTTER $CUTTER_ARGS "$@" $BASE_DIR
