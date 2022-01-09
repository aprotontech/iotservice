<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\Role;
use App\Models\UserRole;
use App\Models\RolePermission;

class RoleDelController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'id' => "is_int",
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $role = Role::find($reqinfo->id);
        if (empty($role)) {
            return $this->error("404", "role({$reqinfo->id}) not found");
        }

        RolePermission::where('role_id', $role->id)->delete();
        UserRole::where('role_id', $role->id)->delete();
        $role->delete();

        return $this->success();
    }
}
