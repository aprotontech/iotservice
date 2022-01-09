<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\Permission;

class PermissionEditController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'id' => "is_int",
            'name' => "is_string",
            'description' => "is_string",
        ];
    }
    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $p = Permission::find($reqinfo->id);
        if (empty($p)) {
            return $this->error("404", "permission({$reqinfo->id}) not found");
        }

        $p->name = trim($reqinfo->name);
        $p->description = trim($reqinfo->description);
        $p->save();

        return $this->success();
    }
}
