/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     processgroup.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-15 10:22:58
 *
 */

#include "common.h"
#include "proton/process/group.h"

/** {{{
 */
PHP_METHOD(processgroup, __construct) {

  proton_coroutine_runtime *runtime = proton_get_runtime();

  proton_process_group_t *group =
      (proton_process_group_t *)qmalloc(sizeof(proton_process_group_t));

  proton_process_group_config_t config;
  memset(&config, 0, sizeof(config));
  config.static_worker_count = 1;

  proton_process_group_init(&group->value, runtime, &config);

  proton_object_construct(getThis(), &group->value);
}
/* }}} */

/** {{{
 */
PHP_METHOD(processgroup, __destruct) { proton_object_destruct(getThis()); }
/* }}} */

/** {{{
 */
PHP_METHOD(processgroup, __toString) { RETURN_STRING("{processgroup}"); }
/* }}} */

/** {{{
 */
PHP_METHOD(processgroup, start) {
  ZEND_PARSE_PARAMETERS_NONE();
  int rc = proton_process_group_start(proton_object_get(getThis()));
  if (rc != 0) {
    php_error_docref(NULL TSRMLS_CC, E_ERROR, "create worker failed");
    return;
  }

  if (proton_process_group_get_type(proton_object_get(getThis())) ==
      PROCESS_MASTER) {
    RETURN_TRUE;
  }
  RETURN_FALSE;
}
/* }}} */

/** {{{
 */
PHP_METHOD(processgroup, stop) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(0);
}
/* }}} */

/* {{{ martin_functions[]
 *
 * Every user visible function must have an entry in processgroup_functions[].
 */
const zend_function_entry processgroup_functions[] = {
    PHP_ME(processgroup, __construct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR) // processgroup::__construct
    PHP_ME(processgroup, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // processgroup::__destruct
    PHP_ME(processgroup, __toString, NULL,
           ZEND_ACC_PUBLIC) // processgroup::__toString
    PHP_ME(processgroup, start, NULL,
           ZEND_ACC_PUBLIC)                           // processgroup::start
    PHP_ME(processgroup, stop, NULL, ZEND_ACC_PUBLIC) // processgroup::stop
    {NULL, NULL, NULL} /* Must be the last line in processgroup_functions[] */
};
/* }}} */

zend_class_entry *_processgroup_ce;

zend_class_entry *regist_processgroup_class() {
  zend_class_entry processgroup;

  INIT_NS_CLASS_ENTRY(processgroup, PROTON_NAMESPACE, "ProcessGroup",
                      processgroup_functions);
  _processgroup_ce = zend_register_internal_class_ex(&processgroup, NULL);

  zend_declare_property_null(_processgroup_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  return _processgroup_ce;
}