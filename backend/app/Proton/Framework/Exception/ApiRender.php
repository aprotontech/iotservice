<?php

namespace Proton\Framework\Exception;

use Symfony\Component\HttpKernel\Exception\NotFoundHttpException;

class ApiRender
{
    public static function toError($request, \Exception $e)
    {
        rclog_exception($e);
        if ($e instanceof NotFoundHttpException) { // not found exception
            $r = rc_error("404", rc_exception_message($e));
            return response($r, 404);
        } else if ($e instanceof ApiException) {
            return response($e->getApiError(), 200)
                ->header('Content-Type', 'application/json');
        }
    }
}
