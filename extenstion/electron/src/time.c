/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     time.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-18 11:38:42
 *
 */

#include "common.h"

/* {{{ void proton_sleep( long $time_ms )
 */
PHP_FUNCTION(proton_sleep) {
  long time_ms = 0;
  quark_coroutine_runtime *runtime = quark_get_runtime();

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(time_ms)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  quark_coroutine_sleep(runtime, time_ms);
}
/* }}}*/