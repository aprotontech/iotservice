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
#include "runtime.h"

// swap to coroutine[dest]
int proton_coroutine_swap_in(proton_coroutine_task *dest) {
  if (dest == NULL || dest == RUNTIME_CURRENT_COROUTINE(dest->runtime)) {
    PLOG_INFO("[COROUTINE] swap failed. input coroutinue can't swap in");
    return -1;
  }

  ucontext_t *org_context =
      &(RUNTIME_CURRENT_COROUTINE(dest->runtime)->context);

  dest->status = QC_STATUS_RUNNING;
  dest->origin = RUNTIME_CURRENT_COROUTINE(dest->runtime);

  if (IS_REAL_COROUTINE(dest->origin)) {
    // add ref to running coroutine(save to [origin]), when [dest] swap out,
    // will swap to [origin]
    Z_TRY_ADDREF(dest->origin->value.myself);
  }

  PLOG_INFO("[COROUTINE] [resume] switch(%lu->%lu)", dest->origin->cid,
            dest->cid);

  RUNTIME_CURRENT_COROUTINE(dest->runtime) = dest;

  // jump to [dest->context], save to [org_context]
  swapcontext(org_context, &(dest->context));
  return 0;
}

// from coroutine[current] swap out
int proton_coroutine_swap_out(proton_coroutine_task *current,
                              proton_coroutine_status new_status) {
  if (current == NULL ||
      current != RUNTIME_CURRENT_COROUTINE(current->runtime)) {
    PLOG_WARN("[COROUTINE] swap failed. input coroutinue can't swap out");
    return -1;
  } else if (current == RUNTIME_MAIN_COROUTINE(current->runtime)) {
    PLOG_WARN("[COROUTINE] swap failed. main coroutinue can't swap out");
    return -1;
  }
  if (new_status != QC_STATUS_RUNABLE && new_status != QC_STATUS_SUSPEND) {
    PLOG_ERROR("[COROUTINE] swap failed. input status must be "
               "QC_STATUS_RUNABLE or QC_STATUS_SUSPEND.");
    return -1;
  }

  assert(current->origin != NULL);

  current->status = new_status;

  // origin coroutinue is waiting for some object or signal
  // so swap to main coroutine(schedule to a runable coroutinue)
  proton_coroutine_status org_status = current->origin->status;
  if (org_status == QC_STATUS_SUSPEND || org_status == QC_STATUS_STOPED) {
    PLOG_INFO(
        "[COROUTINE] origin coroutine is suspend or stoped, so switch to main");
    current->origin = RUNTIME_MAIN_COROUTINE(current->runtime);
  }

  proton_coroutine_task *origin = current->origin;

  PLOG_INFO("[COROUTINE] [yield] switch(%lu->%lu)", current->cid, origin->cid);

  // switch to origin coroutine
  origin->status = QC_STATUS_RUNNING;
  RUNTIME_CURRENT_COROUTINE(current->runtime) = origin;
  current->origin = RUNTIME_MAIN_COROUTINE(current->runtime);

  if (IS_REAL_COROUTINE(origin)) {
    // [current] no need [origin] now, so un-ref it
    RELEASE_VALUE_MYSELF(origin->value);
  }

  // jump to [origin->context], save to [current->context]
  swapcontext(&(current->context), &(origin->context));
  return 0;
}
