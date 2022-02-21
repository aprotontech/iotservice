<?php

namespace Proton\Framework\Response;

class ApiResponse implements \JsonSerializable
{

    private $rc;
    private $err;

    public function __construct($rc = 0, $msg = '')
    {
        $this->rc = $rc;
        $this->err = $msg;
    }

    public function success($data = false)
    {
        $this->rc = 0;
        $this->err = 'success';
        if (!is_array($data)) $data = [];

        foreach ($data as $key => $val) {
            if ($key != 'rc' && $key != 'err') {
                $this->{$key} = $val;
            }
        }

        return $this;
    }

    public function error($rc, $msg)
    {
        $this->rc = $rc;
        $this->err = "$msg";

        return $this;
    }

    public function set()
    {
        $args = func_get_args();
        $length = count($args);
        if ($length % 2 == 0) {
            for ($i = $length; $i > 0; $i -= 2) {
                $key = $args[$i - 2];
                $val = $args[$i - 1];
                if (!is_string($key)) {
                    throw new \Exception("response key is not string. key is " . var_export($key, true));
                }

                $this->$key = $val;
            }
        } elseif ($length == 1 && is_array($args[0])) {
            foreach ($args[0] as $key => $val) {
                $this->$key = $val;
            }
        } else {
            throw new \Exception("response args is invalidate, args=" . var_export($args, true));
        }

        return $this;
    }

    public function getCode()
    {
        return $this->rc;
    }

    public function getErrorMsg()
    {
        return $this->err;
    }

    public function isError()
    {
        return $this->rc !== 0;
    }

    public function isSuccess()
    {
        return $this->rc === 0;
    }

    public function __toString()
    {
        $m = $this->jsonSerialize();
        return json_encode($m);
    }

    public function toArray()
    {
        return get_object_vars($this);
    }

    public function jsonSerialize()
    {
        $m = get_object_vars($this);
        if ($this->rc === 0) {
            unset($m['err']);
        }
        return $m;
    }
};
