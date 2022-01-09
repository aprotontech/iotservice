<?php

function auto_define($key, $value, $redefine_exception = false)
{
    if (!defined($key)) {
        define($key, $value);
        return true;
    } else if ($redefine_exception) {
        $org = constant($key);
        throw new \Exception("refine key($key), current value($org), new value($value)");
    }

    return false;
}

function rc_success($data = false)
{
    return (new \Proton\Framework\Error())->success($data);
}

function rc_error($rc, $msg)
{
    return new \Proton\Framework\Error($rc, $msg);
}

function rpc_client($service)
{
    return \Proton\Framework\Rpc\Client::newDefaultCaller($service);
}

function rc_exception_message($exception)
{
    if (in_array(php_sapi_name(), ['cli', 'cli-server'])) {
        $name = cli_get_process_title();
        if (!$name && isset($argv[1])) $name = $argv[1];
        $more = sprintf(
            "process=%s, pid=%d",
            $name,
            getmypid()
        );
    } else {
        $url = isset($_SERVER['REQUEST_URI']) ? $_SERVER['REQUEST_URI'] : '';
        $more = "url=$url";
    }

    $msg = sprintf(
        "(%s:%d) exception(%s), message(%s). more(%s)",
        $exception->getFile(),
        $exception->getLine(),
        get_class($exception),
        $exception->getMessage(),
        $more
    );

    return $msg;
}

function rc_mstime()
{
    return floor(microtime(true) * 1000);
}

function rc_timestamp()
{
    return floor(microtime(true) * 1000);
}

function rc_exception($rc, $msg)
{
    return new \Proton\Framework\Exception\ApiException($rc, $msg);
}
