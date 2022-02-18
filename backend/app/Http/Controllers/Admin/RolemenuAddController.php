<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\Menu;
use App\Models\Role;
use App\Models\RoleMenu;

class RolemenuAddController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'roleId' => "is_int",
            'menus' => 'is_array',
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $role = Role::find($reqinfo->roleId);

        if (empty($role)) {
            return $this->error("404", "role({$reqinfo->roleId}) not found");
        }

        $menus = $reqinfo->menus;
        $roleId = intval($reqinfo->roleId);
        $system = trim($role->system);

        RoleMenu::where('role_id', $roleId)->delete();
        if ($menus) {
            $arr = [];
            foreach ($menus as $mid) {
                $arr[] = [
                    'menu_id' => $mid,
                    'role_id' => $roleId,
                    'system' => $system,
                    'created_at' => date('Y-m-d H:i:s'),
                    'updated_at' => date('Y-m-d H:i:s'),
                ];
            }
            RoleMenu::insert($arr);
        }

        return $this->success();
    }
}
