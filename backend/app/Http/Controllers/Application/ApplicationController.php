<?php

namespace App\Http\Controllers\Application;

use Illuminate\Http\Request;

class ApplicationController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [];
    }

    public function handle(Request $request)
    {
        $api = new AppListController();
        rclog_debug($request->input('permission'));
        $request->replace([
            'system' => env('PERMISSION_SYSTEM_NAME', 'aproton'),
            'permissions' => [$request->input('permission', 'device_view')]
        ]);
        $r = $api->handle($request);
        $appList = [];
        foreach ($r->apps as $app) {
            $app->appid = $app->appId;
            unset($app->appId);
            array_push($appList, $app);
        }

        return $this->success([
            'appList' => $appList
        ]);
    }
}
