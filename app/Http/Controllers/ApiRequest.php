<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;

class ApiRequest implements \JsonSerializable 
{
    private $rawRequest;
    public function __construct(Request $request, array $checkers)
    {
        $this->rawRequest = $request;

        @ $m = json_decode($request->getContent(), true);

        $msg = $this->check("{REQUEST}", $m, $checkers);
        if ($msg) throw new ApiException($msg);
    }

    private function check($prefix, $input, $checkers)
    {
        foreach ($checkers as $key => $val) {
            if (!array_key_exists($key, $input)) {
                return "$prefix.$key is not found";
            }
            if (is_string($val) && is_callable($val)) {
                $r = call_user_func($val, $input[$key]);
                if ($r) {
                    return "$prefix.$key is not match $val";
                }
            } else if (is_array($val) && is_array($input[$key])) {
                $msg = $this->check("$prefix.$key", $input[$key], $val);
                if ($msg) return $msg;
            }
            
        }

        return true;
    }
}