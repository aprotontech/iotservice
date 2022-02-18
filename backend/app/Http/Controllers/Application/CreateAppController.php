<?php

namespace App\Http\Controllers\Application;

use Illuminate\Http\Request;

use App\Models\Application;

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

        $app = new Application();
        $app->appid = substr(md5(microtime(true) . getmypid() . 'aproton/apps'), 8, 12);
        $app->name = $info->name;
        $app->description = $info->desc;
        $app->creator = $user->id;
        $app->save();

        return $this->success();
    }
}
