<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\Permission;

class PermissionAddController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'permission' => "is_string",
            'name' => "is_string",
            'description' => "is_string",
            'sys' => 'is_string',
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $exists = Permission::where('permission', $reqinfo->permission)
            ->where('system', $reqinfo->sys)
            ->exists();
        if ($exists) {
            return $this->erorr("500", "permission exists");
        }

        $p = new Permission();
        $p->permission = trim($reqinfo->permission);
        $p->name = trim($reqinfo->name);
        $p->description = trim($reqinfo->description);
        $p->system = trim($reqinfo->sys);
        $p->save();

        return $this->success();
    }
}
