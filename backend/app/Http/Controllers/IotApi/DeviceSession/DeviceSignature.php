<?php

namespace App\Http\Controllers\IotApi\DeviceSession;

use Illuminate\Support\Facades\DB;
use Proton\Framework\Crypt\RSA;

class DeviceSignature
{

    public static function auth($device, $sign, $random, $time = false)
    {
        $r = self::deviceAuth($device, $sign, $random, $time);
        if (!$r->isError()) return $r;

        return self::appAuth($device->app_id, $random, $sign, $time);
    }

    public static function deviceAuth($device, $sign, $random, $time = false)
    {
        if ($device->deleted_at) {
            return rc_error(403, "input device($device->client_id) had disable at $device->deleted_at");
        }

        $rsa = new RSA($device->public_key, $device->private_key, $device->salt);
        $decrypt = $rsa->auth($sign, $random);
        if (!$decrypt) {
            if ($device->secret && $time) {
                $s = md5(md5($device->secret . $random . floor($time / 60)) . 'Proton');
                if ($s == $sign) {
                    $decrypt = substr(md5($s), 8, 8);
                }
            }
            if (!$decrypt) {
                return rc_error("1402", "input sign is invalidate");
            }
        }

        return rc_success([
            'secret' => $decrypt
        ]);
    }

    public static function appAuth($appId, $random, $sign, $time = false)
    {
        $appkey = DB::table('app_keys')
            ->where('app_id', $appId)
            ->first();

        if (!$appkey) {
            return rc_error("1100", "not found appId($appId)'s key");
        }

        $rsa = new RSA($appkey->public_key, $appkey->private_key, $appkey->salt);
        $decrypt = $rsa->auth($sign, $random);
        if (!$decrypt) {
            if ($appkey->secret && $time) {
                $s = md5(md5($appkey->secret . $random . floor($time / 60)) . 'aproton.tech');
                if ($s == $sign) {
                    $decrypt = substr(md5($s), 8, 8);
                }
            }
        }

        if (!$decrypt) {
            return rc_error("1101", "decrypt value failed.");
        }

        return rc_success([
            'secret' => $decrypt
        ]);
    }
}
