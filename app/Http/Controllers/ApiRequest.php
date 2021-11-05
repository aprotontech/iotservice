<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;

class ApiRequest implements \JsonSerializable 
{
    private $rawRequest;
    private $jsonContent;
    public function __construct(Request $request, array $checkers)
    {
        $this->rawRequest = $request;

        @ $m = json_decode($request->getContent(), true);

        $msg = $this->check("{REQUEST}", $m, $checkers);
        if (is_string($msg)) throw new ApiException(ErrorCode::EC_INVALIDATE_INPUT, $msg);

        $this->jsonContent = $m;
    }

    public function __get($key)
    {
        if (!$this->jsonContent) {
            throw new ApiException(ErrorCode::EC_ACCESS_INPUT, "input is not json");
        } else if (!array_key_exists($key, $this->jsonContent)) {
            throw new ApiException(ErrorCode::EC_ACCESS_INPUT, "access invalidate input $key");
        }

        return $this->jsonContent[$key];
    }

    private function check($prefix, $input, $checkers)
    {
        if (!empty($checkers) && empty($input)) {
            return "$prefix is not json";
        }
        foreach ($checkers as $key => $val) {
            if (!array_key_exists($key, $input)) {
                return "$prefix.$key is not found";
            }
            if (is_string($val) && is_callable($val)) {
                $r = call_user_func($val, $input[$key]);
                if (!$r) {
                    return "$prefix.$key is not match $val";
                }
            } else if (is_array($val) && is_array($input[$key])) {
                $msg = $this->check("$prefix.$key", $input[$key], $val);
                if ($msg) return $msg;
            }
            
        }

        return true;
    }

    public function jsonSerialize()
    {
        @ $m = json_decode($this->rawRequest->getContent(), true);
        return $m;
    }
}