/* electron extension for PHP */

#ifndef PHP_ELECTRON_H
#define PHP_ELECTRON_H

extern zend_module_entry electron_module_entry;
#define phpext_electron_ptr &electron_module_entry

#define PHP_ELECTRON_VERSION "0.1.0"

#if defined(ZTS) && defined(COMPILE_DL_ELECTRON)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE()                                           \
  ZEND_PARSE_PARAMETERS_START(0, 0)                                            \
  ZEND_PARSE_PARAMETERS_END()
#endif

#endif /* PHP_ELECTRON_H */
