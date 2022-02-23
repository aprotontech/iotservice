<?php

namespace Proton\Framework\Rpc;

interface IMethod
{
    public function __call($name, $arguments);
}
