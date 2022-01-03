<?php

namespace App\Http\Controllers;

use Log;
use Illuminate\Http\Request;

interface IApiRequestChecker
{
    public function check($prefix, $input, $checkers);
}

class ApiRequest implements \JsonSerializable, IApiRequestChecker
{
    private $rawRequest;
    private $jsonContent;
    public function __construct(Request $request, array $checkers)
    {
        $this->rawRequest = $request;

        $m = null;
        switch ($request->method()) {
        case "POST":
            @ $m = json_decode($request->getContent(), true);
            break;
        case "GET":
            $m = $request->all();
            break;
        }

        $msg = ApiRequest::doCheck("{REQUEST}", $m, $checkers);
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

    public function jsonSerialize()
    {
        @ $m = json_decode($this->rawRequest->getContent(), true);
        return $m;
    }

    public function check($prefix, $input, $checkers)
    {
        return $this->doCheck($prefix, $input, $checkers);
    }

    public static function isArrayOf($itemChecker) {
        return new class($itemChecker) implements IApiRequestChecker {
            private $itemChecker;

            public function __construct($ic) {
                $this->itemChecker = $ic;
            }

            public function check($prefix, $input, $checkers) {
                if (!is_array($input)) {
                    return "$prefix is not array.";
                }

                foreach ($input as $idx => $val) {
                    $r = ApiRequest::doCheck(sprintf("%s[%d]", $prefix, $idx), $val, $this->itemChecker);
                    if (is_string($r)) return $r;
                }

                return true;
            }
        };
    }

    public static function doCheck($prefix, $input, $checkers)
    {
        if (!empty($checkers) && empty($input)) {
            return "$prefix is not json";
        }

        if (is_callable($checkers)) {
            $r = call_user_func($checkers, $input);
            if (!$r) {
                return "$prefix is not match $checkers";
            }
        }

        foreach ($checkers as $key => $val) {
            $msg = ApiRequest::doItemCheck($prefix, $input, $key, $val);
            if (is_string($msg)) return $msg;
        }

        return true;
    }

    protected static function doItemCheck($prefix, $input, $key, $checker)
    {
        if (!array_key_exists($key, $input)) {
            return "$prefix.$key is not found";
        }
        if (is_string($checker) && is_callable($checker)) {
            $r = call_user_func($checker, $input[$key]);
            if (!$r) {
                return "$prefix.$key is not match $checker";
            }
        } else if (is_array($checker) && is_array($input[$key])) {
            $msg = $this->check("$prefix.$key", $input[$key], $checker);
            if ($msg) return $msg;
        } else if ($checker instanceof IApiRequestChecker) {
            $msg = $checker->check("$prefix.$key", $input[$key], $checker);
            if ($msg) return $msg;
        } else {
            throw new ApiException(500, "invalidate checker: ".var_export($checker, true));
        }

        return true;
    }

}
