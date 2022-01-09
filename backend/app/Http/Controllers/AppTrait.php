<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\Models\Application;

trait AppTrait
{

    protected function getAllowedAppList($user, $permissions, $system)
    {
        $ownApps = Application::where('creator', $user->id)
            ->select('appid as appId', 'name')
            ->get();

        $apps = [];
        foreach ($ownApps as $app) {
            $apps[$app->appId] = $app;
        }

        if (!isset($permissions)) {
            $permissions = ['app_manager'];
        }

        if (!empty($permissions)) {

            $checker = new Checker();
            $gdata = $checker->getGrantData($user->id, $system, $permissions, ['appId']);
            if (!$gdata) $appIds = [];
            else $appIds = $gdata['appId'];

            $query = Application::select('appid as appId', 'name');
            if (!in_array('*', $appIds)) {
                $query->whereIn('appid', $appIds);
            }
            $grantApps = $query->get();

            foreach ($grantApps as $app) {
                if (!$app->appId) continue;
                $apps[$app->appId] = $app;
            }
        }

        return $this->success([
            'apps' => array_values($apps)
        ]);
    }

    protected function getAppList(Request $request, $appIdList = false)
    {
        $user = $request->user();
        $m = explode(';', $this->permission);
        $permissions = explode(',', $m[0]);
        $system = env('PERMISSION_SYSTEM_NAME', 'aproton');

        $r = $this->getAllowedAppList($user, $permissions, $system);
        if ($r->isError()) {
            throw new \Exception("get allowed applist failed with($r)");
        }

        $list = [];
        foreach ($r->apps as $app) {
            $list[$app['appId']] = (object)[
                'name' => $app['name']
            ];
        }

        if (!$appIdList) return $list;
        $appList = [];
        foreach ($appIdList as $appId) {
            if (isset($list[$appId])) {
                $appList[$appId] = $list[$appId];
            }
        }

        return $appList;
    }
}
