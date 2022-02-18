<?php

namespace App\Exceptions;

use Proton\Framework\Exception\ApiException;

class UserNotLoginException extends ApiException
{
    public function __construct()
    {
        parent::__construct("401", 'user not login');
    }
};
