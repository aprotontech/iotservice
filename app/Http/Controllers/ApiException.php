<?php

namespace App\Http\Controllers;

class ApiException extends \Exception
{
    private $response;

    public function __construct($code, $msg)
    {
        $this->response = new ApiResponse($code, $msg);
    }

    public function __toString()
    {
        return "$this->response";
    }

}