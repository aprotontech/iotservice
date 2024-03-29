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

proton_coroutine_runtime *proton_get_runtime() {
  if (__uv_scheduler != NULL) {
    return &__uv_scheduler->runtime;
  }

  return NULL;
}

void destruct_proton_private_value(zend_resource *rsrc) {
  // PLOG_DEBUG("[ELECTRON] private_value free");
  if (rsrc != NULL && rsrc->ptr != NULL) {
    proton_private_value_t *value = (proton_private_value_t *)rsrc->ptr;
    if (value->type != NULL && value->type->destruct != NULL) {
      PLOG_INFO("[ELECTRON] private_value(%s) free(%p)", value->type->whoami(),
                value);
      if (value->type->destruct(value) == 0) {
        qfree(value);
      }

    } else {
      PLOG_ERROR("[ELECTRON] private_value(%p) invalidate status", value);
    }
  }
}

int proton_object_construct(zval *self, proton_private_value_t *val) {
  PLOG_INFO("proton_object_construct(%s), ptr(%p)",
            ZSTR_VAL(Z_OBJCE_P(self)->name), val);
  zend_resource *r = zend_register_resource(val, pc_private_resource_handle);

  if (r == NULL) {
    PLOG_ERROR("register resource failed");
    return -1;
  }

  zval value;
  ZVAL_RES(&value, r);

  zend_update_property(Z_OBJCE_P(self), self,
                       ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                       &value TSRMLS_CC);

  ZVAL_COPY(&val->myself, self);
  ZVAL_PTR_DTOR(&value); // remove value's ref-count
  return 0;
}

int proton_object_destruct(zval *self) {
  PLOG_INFO("proton_object_destruct(%s)", ZSTR_VAL(Z_OBJCE_P(self)->name));

  zval value;
  ZVAL_NULL(&value);

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
      PLOG_WARN("[ELECTRON] get %s faild", PHP_PRIVATE_VALUE_RESOURCE_NAME);
    }

    return ptr;
  }

  return NULL;
}

const char **get_input_headers(zval *headers, int *output_headers_count) {
  if (headers != NULL) {
    HashTable *array_hash = HASH_OF(headers);
    int input_headers_count = zend_array_count(array_hash);
    if (input_headers_count != 0) {
      zend_ulong num_idx, valid_idx = 0;
      zend_string *str_idx;
      zval *entry;
      const char **input_headers =
          (const char **)malloc(sizeof(char *) * input_headers_count);
      ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(headers), num_idx, str_idx, entry) {
        if (Z_TYPE_P(entry) != IS_STRING) {
          php_error_docref(NULL TSRMLS_CC, E_WARNING,
                           "input headers item must be string");
          free(input_headers);
          return NULL;
        }

        zend_string *value = Z_STR_P(entry);
        input_headers[valid_idx++] = value->val;
      }
      ZEND_HASH_FOREACH_END();

      *output_headers_count = valid_idx;

      return input_headers;
    }
  }

  return NULL;
}