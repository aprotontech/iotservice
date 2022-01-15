/* electron extension for PHP */

#ifndef PHP_ELECTRON_H
# define PHP_ELECTRON_H

extern zend_module_entry electron_module_entry;
# define phpext_electron_ptr &electron_module_entry

# define PHP_ELECTRON_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_ELECTRON)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#define PHP_COROUTINE_RESOURCE_NAME "proton_coroutine"

#endif	/* PHP_ELECTRON_H */
