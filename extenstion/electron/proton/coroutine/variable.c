/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     variable.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-16 04:57:59
 *
 */

#include "task.h"

#include "ext/standard/info.h"
#include "php.h"

quark_coroutine_var_t *search_var(quark_coroutine_task *current,
                                  const char *key,
                                  quark_coroutine_task **ptask) {
  any_t val = NULL;
  quark_coroutine_var_t *var = NULL;

  quark_coroutine_task *task = current;

  while (task != NULL) {
    if (task->vars != NULL &&
        hashmap_get(task->vars, (char *)key, &val) == MAP_OK) {
      assert(val != NULL);
      var = (quark_coroutine_var_t *)val;
      *ptask = task;
      break;
    }

    task = task->parent;
  }

  return var;
}

quark_coroutine_var_t *copy_var_to_current(quark_coroutine_task *current,
                                           quark_coroutine_var_t *var) {
  int len = strlen(var->key);
  quark_coroutine_var_t *tmp =
      (quark_coroutine_var_t *)qmalloc(sizeof(quark_coroutine_var_t) + len);
  strcpy(tmp->key, var->key);
  ZVAL_COPY(&tmp->val, &var->val);
  tmp->mode = var->mode;
  tmp->set_by_me = 0;
  if (current->vars == NULL) {
    current->vars = hashmap_new();
  }
  hashmap_put(current->vars, tmp->key, tmp);

  return tmp;
}

int quark_coroutine_get_var(quark_coroutine_task *current, const char *key,
                            zval *result) {
  quark_coroutine_task *task = NULL;
  quark_coroutine_var_t *var = search_var(current, key, &task);

  if (var == NULL) {
    QUARK_LOGGER("[COROUTINE] [VAR] not found var(%s)", key);
    return -1;
  }

  if (task != current) { // append to current cache
    copy_var_to_current(current, var);
  }

  if (result != NULL) {
    ZVAL_COPY(result, &var->val);
  }

  return 0;
}

int quark_coroutine_set_var(quark_coroutine_task *current, const char *key,
                            zval *val, quark_coroutine_var_mode mode) {
  quark_coroutine_task *task = NULL;
  quark_coroutine_var_t *var = search_var(current, key, &task);

  if (var != NULL) {
    if (task != current &&
        (var->mode == QC_VAR_READONLY || var->mode == QC_VAR_INHERIT)) {
      QUARK_LOGGER("[COROUTINE] [VAR] can't set var(%s)", key);
      return -1;
    }

    if (current != task) {
      var = copy_var_to_current(current, var);
    }

    ZVAL_COPY(&var->val, val);
  } else { // not found var, so create a new one
    quark_coroutine_var_t tmp = {
        .mode = mode,
        .set_by_me = 1,
        .val = *val,
    };
    copy_var_to_current(current, &tmp);
  }

  return 0;
}