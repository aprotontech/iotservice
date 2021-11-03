<?php

namespace App\Http\Controllers;

class ApiResponse implements \JsonSerializable 
{
    private $data;

    public function __construct($code = 200, $msg = '')
    {
        $this->data = [
            'rc' => $code,
        ];
        if (!$msg) {
            switch ($code)
            {
            case 404: $msg = 'Not Found'; break;

            }
        }

        if ($msg) {
            $this->data['msg'] = $msg;
        }
    }

    public function set()
    {
        $args = func_get_args();
        $length = count($args);
        if ($length % 2 == 0) {
            for ($i = $length; $i > 0; $i -= 2) {
                $key = $args[$i - 2];
                $val = $args[$i - 1];
                if (!is_string($key)) {
                    throw new \Exception("response key is not string. key is ".var_export($key, true));
                }
                $this->data[$key] = $val;
            }
        } elseif ($length == 1 && is_array($args[0])) {
            foreach ($args[0] as $key => $val) {
                $this->data[$key] = $val;
            }
        } else {
            throw new \Exception("response args is invalidate, args=".var_export($key, true));
        }

        return $this;
    }

    public function jsonSerialize()
    {
        return $this->data;
    }

    public function __toString()
    {
        return json_encode($this->data);
    }
}