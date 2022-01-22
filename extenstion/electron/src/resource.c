/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     resource.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-18 11:41:55
 *
 */

#include "common.h"

void destruct_proton_private_value(zend_resource *rsrc) {
  QUARK_LOGGER("[ELECTRON] private_value destruct");
  qfree(rsrc->ptr);
}

int proton_object_construct(zval *self, proton_private_value_t *val) {
  QUARK_LOGGER("proton_object_construct");
  zend_resource *r = zend_register_resource(val, pc_private_resource_handle);

  zval value;
  ZVAL_RES(&value, r);

  zend_update_property(Z_OBJCE_P(self), self,
                       ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                       &value TSRMLS_CC);
  return 0;
}

proton_private_value_t *proton_object_get(zval *self) {
  zval rv;
  zval *value = zend_read_property(Z_OBJCE_P(self), self,
                                   ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                                   0 TSRMLS_CC, &rv);
  if (value != NULL) {
    proton_private_value_t *ptr =
        (proton_private_value_t *)zend_fetch_resource_ex(
            value, PHP_PRIVATE_VALUE_RESOURCE_NAME, pc_private_resource_handle);
    if (ptr == NULL) {
      QUARK_LOGGER("[ELECTRON] get %s faild", PHP_PRIVATE_VALUE_RESOURCE_NAME);
    }

    return ptr;
  }

  return NULL;
}