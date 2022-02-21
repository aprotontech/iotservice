<?php

namespace App\Http\Controllers\Application;

use Illuminate\Support\Facades\DB;
use Illuminate\Http\Request;

use App\Models\Application;

use Proton\Framework\Crypt\Utils as CryptUtils;

class CreateAppController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'name' => 'is_string & is_not_empty',
            'desc' => 'is_string & is_not_empty',
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);
        $user = $this->webApiAuth();

        DB::transaction(function () use ($info, $user) {
            $app = new Application();
            $app->appid = substr(md5(microtime(true) . getmypid() . 'aproton/apps'), 8, 12);
            $app->name = $info->name;
            $app->description = $info->desc;
            $app->creator = $user->id;
            $app->save();

            list($pubKey, $priKey) = CryptUtils::allocRsaKeyPair();

            DB::table('app_keys')
                ->insert([
                    'app_id' => $app->appid,
                    'salt' => '',
                    'public_key' => $pubKey,
                    'private_key' => $priKey,
                    'secret' => md5($pubKey . $priKey . 'aproton.tech' . microtime(true) . getmypid())
                ]);
        });



        return $this->success();
    }
}
