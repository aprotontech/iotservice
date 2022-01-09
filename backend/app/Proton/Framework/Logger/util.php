<?php

function rclog_debug()
{
    if (func_num_args() == 1) Log::debug(func_get_arg(0));
    else Log::debug(call_user_func_array('sprintf', func_get_args()));
}

function rclog_info()
{
    if (func_num_args() == 1) Log::info(func_get_arg(0));
    else Log::info(call_user_func_array('sprintf', func_get_args()));
}

function rclog_warning()
{
    if (func_num_args() == 1) Log::warning(func_get_arg(0));
    else Log::warning(call_user_func_array('sprintf', func_get_args()));
}

function rclog_error()
{
    if (func_num_args() == 1) Log::error(func_get_arg(0));
    else Log::error(call_user_func_array('sprintf', func_get_args()));
}

function rclog_notice()
{
    if (func_num_args() == 1) Log::notice(func_get_arg(0));
    else Log::notice(call_user_func_array('sprintf', func_get_args()));
}

function rclog_exception($exception, $log_as_error = true)
{
    $msg = rc_exception_message($exception);
    rclog_error($msg);
}
