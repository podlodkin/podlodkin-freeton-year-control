install_dir(${TEST_DIR}/libsimple TARGETS check)
install_dir(${TEST_DIR}/libbasic TARGETS check)
test_check_package(NAME simple HEADER simple.h TARGET simple)
test_check_pkgconfig(NAME simple HEADER simple.h)
test_check_package(NAME basic HEADER simple.h TARGET basic)
test_check_pkgconfig(NAME basic HEADER simple.h)
