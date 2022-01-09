<?php

namespace App\Http\Controllers\Application;

use Illuminate\Http\Request;

use App\Models\Application;
use App\Models\User;

class AppListController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'system' => is_optional_of('is_string'),
            'permissions' => is_optional_of(is_array_of('is_string & is_not_empty')),

            'token' => is_optional_of('is_string'),
            'phone' => is_optional_of('is_string'),
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);
        $user = $this->webApiAuth(true);

        $ownApps = Application::where('creator', $user->id)
            ->select('appid as appId', 'name', 'creator', 'created_at')
            ->get();

        $apps = [];
        foreach ($ownApps as $app) {
            $apps[$app->appId] = $app;
        }

        if (!isset($info->permissions)) {
            $info->permissions = ['app_manager'];
        }

        if (!empty($info->permissions)) {
            $system = isset($info->system) ? $info->system : env('PERMISSION_SYSTEM_NAME', 'aproton');

            $checker = new \App\Http\Controllers\Checker();
            $gdata = $checker->getGrantData($user->id, $system, $info->permissions, ['appId']);
            if (!$gdata) $appIds = [];
            else $appIds = $gdata['appId'];

            $query = Application::select('appid as appId', 'name', 'creator', 'created_at');
            if (!in_array('*', $appIds)) {
                $query->whereIn('appid', $appIds);
            }
            $grantApps = $query->get();

            foreach ($grantApps as $app) {
                if (!$app->appId) continue;
                $apps[$app->appId] = $app;
            }
        }

        foreach ($apps as $appId => $app) {
            $user = User::find($app->creator);
            $app->creator = $user->name;
        }

        return $this->success([
            'apps' => array_values($apps)
        ]);
    }
}
