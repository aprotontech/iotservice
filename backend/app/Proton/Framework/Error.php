<?php

namespace Proton\Framework;

class Error implements \JsonSerializable
{

    private $rc;
    private $err;

    public function __construct($rc = '0', $msg = '')
    {
        $this->rc = "$rc";
        $this->err = $msg;
    }

    public function success($data = false)
    {
        $this->rc = '0';
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
        $this->rc = "$rc";
        $this->err = "$msg";

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
        return $this->rc !== "0";
    }

    public function isSuccess()
    {
        return $this->rc === "0";
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
        if ($this->rc === "0") {
            unset($m['err']);
        } else if (defined('NOT_SHOW_ERROR') && NOT_SHOW_ERROR) {
            if ($this->isError()) unset($m['err']);
        }
        return $m;
    }
};
