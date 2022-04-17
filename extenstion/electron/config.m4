dnl config.m4 for extension electron

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary.

dnl If your extension references something external, use 'with':

dnl PHP_ARG_WITH([electron],
dnl   [for electron support],
dnl   [AS_HELP_STRING([--with-electron],
dnl     [Include electron support])])

dnl Otherwise use 'enable':

PHP_ARG_ENABLE([electron],
  [whether to enable electron support],
  [AS_HELP_STRING([--enable-electron],
    [Enable electron support])],
  [no])

if test "$PHP_ELECTRON" != "no"; then
  dnl Write more examples of tests here...

  dnl Remove this code block if the library does not support pkg-config.
  dnl PKG_CHECK_MODULES([LIBFOO], [foo])
  dnl PHP_EVAL_INCLINE($LIBFOO_CFLAGS)
  dnl PHP_EVAL_LIBLINE($LIBFOO_LIBS, ELECTRON_SHARED_LIBADD)

  dnl If you need to check for a particular library version using PKG_CHECK_MODULES,
  dnl you can use comparison operators. For example:
  dnl PKG_CHECK_MODULES([LIBFOO], [foo >= 1.2.3])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo < 3.4])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo = 1.2.3])

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-electron -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/electron.h"  # you most likely want to change this
  dnl if test -r $PHP_ELECTRON/$SEARCH_FOR; then # path given as parameter
  dnl   ELECTRON_DIR=$PHP_ELECTRON
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for electron files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       ELECTRON_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$ELECTRON_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the electron distribution])
  dnl fi

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-electron -> add include path
  dnl PHP_ADD_INCLUDE($ELECTRON_DIR/include)

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-electron -> check for lib and symbol presence
  dnl LIBNAME=ELECTRON # you may want to change this
  dnl LIBSYMBOL=ELECTRON # you most likely want to change this

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   AC_DEFINE(HAVE_ELECTRON_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your electron library.])
  dnl ], [
  dnl   $LIBFOO_LIBS
  dnl ])

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are not using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $ELECTRON_DIR/$PHP_LIBDIR, ELECTRON_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_ELECTRON_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your electron library.])
  dnl ],[
  dnl   -L$ELECTRON_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(ELECTRON_SHARED_LIBADD)

  PHP_ADD_LIBRARY_WITH_PATH(uv, /usr/local/lib, ELECTRON_SHARED_LIBADD)
  PHP_SUBST(ELECTRON_SHARED_LIBADD)

  dnl In case of no dependencies
  AC_DEFINE(HAVE_ELECTRON, 1, [ Have electron support ])

  PHP_NEW_EXTENSION(electron, electron.c src/*.c proton/*/*.c proton/*mqtt/third/*.c, $ext_shared)
  PHP_ADD_INCLUDE([proton/include])
  PHP_ADD_INCLUDE([/usr/local/include])
  dnl PHP_ADD_INCLUDE([/usr/local/paho.mqtt.c/include])
fi
