<?php

namespace Proton\Framework\Request;

interface IRequest
{
    public function reqToObject();

    public function getClientIp();

    public function getHeader($key);

    public function getCookie($key);
}
