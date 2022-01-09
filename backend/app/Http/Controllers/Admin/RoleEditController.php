<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\Role;
use App\Models\RolePermission;

class RoleEditController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'id' => "is_int",
            'name' => 'optional | is_string',
            'desc' => 'optional | is_string',
            'permissions' => 'optional | is_array',
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $permissions = isset($reqinfo->permissions) ? $reqinfo->permissions : null;
        $name = isset($reqinfo->name) ? trim($reqinfo->name) : '';
        $desc = isset($reqinfo->desc) ? trim($reqinfo->desc) : '';

        $role = Role::find($reqinfo->id);
        if (empty($role)) {
            return $this->error("404", "role({$reqinfo->id}) not found");
        }

        if ($name || $desc) {
            $role->name = $name;
            $role->desc = $desc;
            $role->save();
        }

        if ($permissions !== null) {
            RolePermission::where('role_id', $role->id)->delete();
        }

        if ($permissions) {
            $arr = [];
            foreach ($permissions as $permission) {
                array_push($arr, [
                    'role_id' => $role->id,
                    'permission' => $permission,
                    'data_rule' => '*',
                    'system' => $role->system,
                    'created_at' => date('Y-m-d H:i:s'),
                    'updated_at' => date('Y-m-d H:i:s'),
                ]);
            }
            RolePermission::insert($arr);
        }

        return $this->success();
    }
}
