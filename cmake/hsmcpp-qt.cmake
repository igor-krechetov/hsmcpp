pkg_check_modules(HSMCPP_QT REQUIRED hsmcpp_qt)

set(HSMCPP_INCLUDE_DIRS ${HSMCPP_INCLUDE_DIRS} ${HSMCPP_QT_INCLUDE_DIRS})
set(HSMCPP_LDFLAGS ${HSMCPP_LDFLAGS} ${HSMCPP_QT_LDFLAGS})
set(HSMCPP_CFLAGS_OTHER ${HSMCPP_CFLAGS_OTHER} ${HSMCPP_QT_CFLAGS_OTHER})