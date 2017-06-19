PHP_INCLUDE = `php-config --includes`
PHP_LIBS = `php-config --libs`
PHP_LDFLAGS = `php-config --ldflags`
PHP_INCLUDE_DIR = `php-config --include-dir`
PHP_EXTENSION_DIR = `php-config --extension-dir`

swoole_ext.so: swoole_ext.cc pool.cc
	c++ -g -o swoole_ext.so -O0 -fPIC -shared swoole_ext.cc pool.cc -fPIC -shared -std=c++11 ${PHP_INCLUDE} -I${PHP_INCLUDE_DIR} -lphpx\
	 -I${PHP_INCLUDE_DIR}/ext/swoole/include -I${PHP_INCLUDE_DIR}/ext/swoole
install: swoole_ext.so
	cp swoole_ext.so ${PHP_EXTENSION_DIR}/
clean:
	rm *.so
