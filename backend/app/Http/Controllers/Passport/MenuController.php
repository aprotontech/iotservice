<?php

namespace App\Http\Controllers\Passport;

use Illuminate\Http\Request;

use DB;

class MenuController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'mainMenuId' => 'is_string',
            'system' => 'is_string',
            'depth' => 'optional | is_int'
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);
        $user = $this->webApiAuth(false);

        if (!$user) {
            return $this->error(403, "not found webapi token");
        }

        $menus = $this->getMenus($user->id, 0, $info->system);
        $menus = $this->formatTreeMenu($menus, $info->mainMenuId);
        if (isset($info->depth) && $info->depth > 0) {
        }

        return $this->success([
            'menus' => $menus
        ]);
    }

    private function getMenus($userId, $type, $system)
    {
        $menus = [];
        $roleIds = DB::table('user_roles')
            ->where('user_id', $userId)
            ->where('system', $system)
            ->pluck('role_id');

        $roleMenus = DB::table('role_menus')
            ->whereIn('role_id', $roleIds)
            ->where('system', $system)
            ->pluck('menu_id');

        $menus = DB::table('menus')->whereIn('id', $roleMenus)
            ->where('type', $type)
            ->orderBy('sort', 'asc')
            ->orderBy('id', 'asc')
            ->get();

        return $menus;
    }

    private function formatTreeMenu($menus, $mainMenuId)
    {
        $menuNodes = [];
        $root = (object)[
            'children' => []
        ];

        $menuNodes[0] = $root;

        $subMenus = [];
        foreach ($menus as $menu) {
            $menuNodes[$menu->id] = (object)[
                'id' => $menu->id,
                'url' => $menu->url,
                'title' => $menu->title,
                'attrs' => $menu->attrs ? @json_decode($menu->attrs) : (object)[],
                'children' => [],
            ];

            $subMenus[$menu->pid][] = $menu->id;
        }

        foreach ($subMenus as $pid => $cids) {
            if (!isset($menuNodes[$pid])) continue;
            foreach ($cids as $id) {
                array_push($menuNodes[$pid]->children, $menuNodes[$id]);
            }
        }

        if (!$mainMenuId || $mainMenuId == 'root') {
            return $root->children;
        }

        foreach ($menuNodes as $m) {
            if (isset($m->attrs->mainMenuId) && $m->attrs->mainMenuId == $mainMenuId) {
                return $m->children;
            }
        }

        return [];
    }
}
