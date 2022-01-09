<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\Permission;
use App\Models\RolePermission;
use App\Models\ApiConfig;

class PermissionDelController extends \App\Http\Controllers\WebController
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

        $p = Permission::find($reqinfo->id);
        if (empty($p)) {
            return $this->error("404", "permission({$reqinfo->id}) not found");
        }

        RolePermission::where('permission', $p->permission)
            ->where('system', $p->system)
            ->delete();
        //        ApiConfig::where('permission', $p->permission)
        //            ->where('system', $p->system)
        //            ->delete();
        $p->delete();

        return $this->success();
    }
}
