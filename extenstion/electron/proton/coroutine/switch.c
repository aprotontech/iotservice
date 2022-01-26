/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     switch.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-16 05:00:06
 *
 */

#include "task.h"

#include "ext/standard/info.h"
#include "php.h"

quark_coroutine_task *_current_coroutine = NULL;

extern int _is_real_coroutine(quark_coroutine_task *task);

ucontext_t *_get_current_ucontext() {
  assert(_current_coroutine != NULL);

  return &_current_coroutine->context;
}

int quark_coroutine_swap_in(quark_coroutine_task *dest) {
  if (dest == NULL || dest == _current_coroutine) {
    QUARK_LOGGER("[COROUTINE] swap failed. input coroutinue can't swap in");
    return -1;
  }

  ucontext_t *org_context = _get_current_ucontext();

  dest->status = QC_STATUS_RUNNING;
  dest->origin = _current_coroutine;

  if (_is_real_coroutine(_current_coroutine)) {
    // add ref to running coroutine(save to [origin]), when [dest] swap out,
    // will swap to [origin]
    Z_TRY_ADDREF(_current_coroutine->value.myself);
  }

  QUARK_LOGGER("[COROUTINE] [resume] switch(%lu->%lu)", _current_coroutine->cid,
               dest->cid);

  _current_coroutine = dest;

  // jump to [dest->context], save to [org_context]
  swapcontext(org_context, &(dest->context));
  return 0;
}

int quark_coroutine_swap_out(quark_coroutine_task *current,
                             quark_coroutine_status new_status) {
  if (current == NULL || current != _current_coroutine) {
    QUARK_LOGGER("[COROUTINE] swap failed. input coroutinue can't swap out");
    return -1;
  } else if (current == quark_coroutine_get_main()) {
    QUARK_LOGGER("[COROUTINE] swap failed. main coroutinue can't swap out");
    return -1;
  }
  if (new_status != QC_STATUS_RUNABLE && new_status != QC_STATUS_SUSPEND) {
    QUARK_LOGGER("[COROUTINE] swap failed. input status must be "
                 "QC_STATUS_RUNABLE or QC_STATUS_SUSPEND.");
    return -1;
  }

  assert(current->origin != NULL);

  current->status = new_status;

  // origin coroutinue is waiting for some object or signal
  // so swap to main coroutine(schedule to a runable coroutinue)
  quark_coroutine_status org_status = current->origin->status;
  if (org_status == QC_STATUS_SUSPEND || org_status == QC_STATUS_STOPED) {
    QUARK_LOGGER(
        "[COROUTINE] origin coroutine is suspend or stoped, so switch to main");
    current->origin = quark_coroutine_get_main();
  }

  QUARK_LOGGER("[COROUTINE] [yield] switch(%lu->%lu)", current->cid,
               current->origin->cid);

  current->origin->status = QC_STATUS_RUNNING;
  _current_coroutine = current->origin; // switch to origin coroutine
  current->origin = quark_coroutine_get_main();

  if (_is_real_coroutine(_current_coroutine)) {
    // [current] no need [origin] now, so un-ref it
    RELEASE_MYSELF(_current_coroutine->value.myself);
  }

  // jump to [_get_current_ucontext()], save to [current->context]
  swapcontext(&(current->context), _get_current_ucontext());
  return 0;
}
