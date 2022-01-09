<?php

namespace App\Http\Controllers\Admin;

use App\Models\Menu;
use App\Models\RoleMenu;
use App\Models\Role;
use Illuminate\Http\Request;
use Illuminate\Pagination\Paginator;

class RolemenuListController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'id' => 'is_int',
            "sys" => "optional | is_string",
            "page" => "optional | is_int",
            "size" => "optional | is_int",
        ];
    }

    /**
     *  menus = [
     *      {
     *          "type" : "default",
     *          "menus" [
     *              {
     *                  "title" : "root1",
     *                  "key" : "1",
     *                  "selected" : false,
     *                  "children" : [
     *                      {
     *                          "title" : 'child1',
     *                          "key" : "2"
     *                      },
     *                  ]
     *              }
     *          ]
     *      }
     *  ]
     */
    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $role = Role::find($reqinfo->id);

        if (empty($role)) {
            return $r->error("404", "role({$reqinfo->id}) not found");
        }

        $menus = Menu::where('system', $role->system)
            ->orderBy('sort', 'asc')
            ->get();

        $nodeMenus = $this->formatTreeMenu($menus);

        $roleMenus = RoleMenu::where('role_id', $role->id)->get();

        $data = [];
        $data['name'] = $role->name;
        $data['system'] = $role->system;
        $data['id'] = $role->id;
        $data['menus'] = [];

        $rmenus = [];
        foreach ($roleMenus as $menu) {
            if (isset($nodeMenus[$menu->menu_id])) {
                $nodeMenus[$menu->menu_id]->checked = true;
            }
        }

        do {
            $hasUpdate = false;
            foreach ($nodeMenus as $menu) {
                if (!$menu->checked || !$menu->children) continue;
                foreach ($menu->children as $c) {
                    if (!$c->checked && $menu->checked) {
                        $hasUpdate = true;
                        $menu->checked = false;
                        break;
                    }
                }
            }
        } while ($hasUpdate);

        $data['menus'] = $nodeMenus[0];

        return $this->success($data);
    }

    private function formatTreeMenu($menus)
    {
        $menuNodes = [];
        $root = (object)[
            'key' => 0,
            'title' => 'root',
            'type' => 'default',
            'expand' => true,
            'checked' => false,
            'children' => []
        ];

        $menuNodes[0] = $root;

        $subMenus = [];
        foreach ($menus as $menu) {
            $menuNodes[$menu->id] = (object)[
                'key' => $menu->id,
                'title' => $menu->title,
                'expand' => true,
                'children' => [],
                'checked' => false
            ];

            $subMenus[$menu->pid][] = $menu->id;
        }

        foreach ($subMenus as $pid => $cids) {
            if (!isset($menuNodes[$pid])) continue;
            foreach ($cids as $id) {
                array_push($menuNodes[$pid]->children, $menuNodes[$id]);
            }
        }

        return $menuNodes;
    }
}
