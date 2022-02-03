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

#include "variable.h"
#include "task.h"

proton_coroutine_var_t *search_var(proton_coroutine_task *current,
                                   const char *key,
                                   proton_coroutine_task **ptask) {
  any_t val = NULL;
  proton_coroutine_var_t *var = NULL;

  proton_coroutine_task *task = current;

  while (task != NULL) {
    if (task->vars != NULL &&
        hashmap_get(task->vars, (char *)key, &val) == MAP_OK) {
      assert(val != NULL);
      var = (proton_coroutine_var_t *)val;
      *ptask = task;
      break;
    }

    task = task->parent;
  }

  return var;
}

proton_coroutine_var_t *copy_var_to_current(proton_coroutine_task *current,
                                            proton_coroutine_var_t *var) {
  int len = strlen(var->key);
  proton_coroutine_var_t *tmp =
      (proton_coroutine_var_t *)qmalloc(sizeof(proton_coroutine_var_t) + len);
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

int proton_coroutine_get_var(proton_coroutine_task *current, const char *key,
                             zval *result) {
  proton_coroutine_task *task = NULL;
  proton_coroutine_var_t *var = search_var(current, key, &task);

  if (var == NULL) {
    PLOG_WARN("[COROUTINE] [VAR] not found var(%s)", key);
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

int proton_coroutine_set_var(proton_coroutine_task *current, const char *key,
                             zval *val, proton_coroutine_var_mode mode) {
  proton_coroutine_task *task = NULL;
  proton_coroutine_var_t *var = search_var(current, key, &task);

  if (var != NULL) {
    if (task != current &&
        (var->mode == QC_VAR_READONLY || var->mode == QC_VAR_INHERIT)) {
      PLOG_WARN("[COROUTINE] [VAR] can't set var(%s)", key);
      return -1;
    }

    if (current != task) {
      var = copy_var_to_current(current, var);
    }

    ZVAL_COPY(&var->val, val);
  } else { // not found var, so create a new one
    proton_coroutine_var_t tmp = {
        .mode = mode,
        .set_by_me = 1,
        .val = *val,
    };
    copy_var_to_current(current, &tmp);
  }

  return 0;
}

int _free_var_item(any_t item, const char *key, any_t data) {
  proton_coroutine_var_t *var = (proton_coroutine_var_t *)item;
  ZVAL_PTR_DTOR(&var->val);
  qfree(var);
  return MAP_OK;
}