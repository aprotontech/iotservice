<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\Role;

class RoleAddController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'name' => 'is_string',
            //            'type' => 'is_int',
            'sys' => 'is_string',
            'desc' => 'is_string',
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $role = new Role();
        $role->name = trim($reqinfo->name);
        //        $role->type = intval($reqinfo->type);
        $role->system = trim($reqinfo->sys);
        $role->desc = trim($reqinfo->desc);

        $role->save();
        return $this->success();
    }
}
