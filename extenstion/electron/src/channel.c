/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     channel.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-17 01:03:26
 *
 */

#include "common.h"

/** {{{
 */
PHP_METHOD(channel, __construct) {
  proton_coroutine_runtime *runtime = proton_get_runtime();

  long max_size;
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(max_size)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  proton_channel_t *channel =
      (proton_channel_t *)qmalloc(sizeof(proton_channel_t));
  if (proton_channel_init(runtime, channel, max_size) != 0) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "init channel failed");
    qfree(channel);
    return;
  }

  proton_object_construct(getThis(), &channel->value);
}
/* }}} */

/** {{{
 */
PHP_METHOD(channel, __destruct) { proton_object_destruct(getThis()); }
/* }}} */

/** {{{
 */
PHP_METHOD(channel, __toString) { RETURN_STRING("{channel}"); }
/* }}} */

/** {{{
 */
PHP_METHOD(channel, push) {
  zval *item = NULL;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(item)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  RETURN_LONG(proton_channel_push(proton_object_get(getThis()), item));
}
/* }}} */

/** {{{
 */
PHP_METHOD(channel, pop) {
  zval item;

  if (proton_channel_pop(proton_object_get(getThis()), &item) == 0) {
    RETURN_ZVAL(&item, 0, 0);
  }

  RETURN_FALSE;
}
/* }}} */

/** {{{
 */
PHP_METHOD(channel, close) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(proton_channel_close(proton_object_get(getThis())));
}
/* }}} */

/* {{{ martin_functions[]
 *
 * Every user visible function must have an entry in channel_functions[].
 */
const zend_function_entry channel_functions[] = {
    PHP_ME(channel, __construct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR) // channel::__construct
    PHP_ME(channel, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // channel::__destruct
    PHP_ME(channel, __toString, NULL,
           ZEND_ACC_PUBLIC)                       // channel::__toString
    PHP_ME(channel, push, NULL, ZEND_ACC_PUBLIC)  // channel::push
    PHP_ME(channel, pop, NULL, ZEND_ACC_PUBLIC)   // channel::pop
    PHP_ME(channel, close, NULL, ZEND_ACC_PUBLIC) // channel::close
    {NULL, NULL, NULL} /* Must be the last line in channel_functions[] */
};
/* }}} */

zend_class_entry *_channel_ce;

zend_class_entry *regist_channel_class() {
  zend_class_entry channel;

  INIT_NS_CLASS_ENTRY(channel, PROTON_NAMESPACE, "Channel", channel_functions);
  _channel_ce = zend_register_internal_class_ex(&channel, NULL);

  zend_declare_property_null(_channel_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  return _channel_ce;
}