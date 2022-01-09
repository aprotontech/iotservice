<?php

namespace Proton\Framework\Exception;

use Exception;

class ApiException extends Exception
{
    private $err;

    public function __construct($errcode, $errmsg)
    {
        $this->err = rc_error($errcode, $errmsg);
        parent::__construct($errmsg, $errcode);
    }

    public function getApiError()
    {
        return $this->err;
    }
}
