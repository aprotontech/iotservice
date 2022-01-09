<?php

namespace App\Http\Controllers;

use Illuminate\Foundation\Auth\Access\AuthorizesRequests;
use Illuminate\Foundation\Bus\DispatchesJobs;
use Illuminate\Foundation\Validation\ValidatesRequests;
use Illuminate\Routing\Controller as BaseController;

class Controller extends BaseController
{
    use AuthorizesRequests, DispatchesJobs, ValidatesRequests;

    //use \Proton\Framework\ApiControllerTrait;

    /*protected function apiError($errorCode, $message)
    {
        return [
            "error" => "$errorCode",
            "message" => $message
        ];
    }

    protected function apiSuccess($data = false)
    {
        if (!$data) $m = [];
        else if (is_array($data)) $m = $data;
        else if (is_object($data)) $m = (array)$data;
        else throw new \Exception("invalidate response data");

        $m['error'] = "success";
        return $m;
    }*/
}
