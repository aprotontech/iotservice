<?php

namespace App\Http\Controllers\Application;

use Illuminate\Http\Request;

use DB;
use Proton\Framework\Crypt\RSA;

class AppDecryptController extends \App\Http\Controllers\Controller
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'appId' => 'is_string & is_not_empty',
            'encrypt' => 'is_string & is_not_empty',
            'random' => 'is_string',
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);

        $appkey = DB::table('app_keys')
            ->where('app_id', $info->appId)
            ->first();

        if (!$appkey) {
            return $this->error("1100", "not found appId($info->appId)'s key");
        }

        $rsa = new RSA($appkey->public_key, $appkey->private_key, $appkey->salt);
        $decrypt = $rsa->auth($info->encrypt, $info->random);
        if (!$decrypt) {
            if ($appkey->secret && isset($info->time)) {
                $s = md5(md5($appkey->secret . $info->random . floor($info->time / 60)) . 'APROTON');
                if ($s == $info->encrypt) {
                    $decrypt = substr(md5($s), 8, 8);
                }
            }
        }

        if (!$decrypt) {
            return $this->error("1101", "decrypt value failed.");
        }

        return $this->success([
            'value' => $decrypt
        ]);
    }
}
