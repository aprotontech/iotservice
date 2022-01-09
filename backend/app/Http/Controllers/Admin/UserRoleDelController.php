<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\User;
use App\Models\Role;
use App\Models\UserRole;
use App\Models\GrantData;

class UserRoleDelController extends \App\Http\Controllers\WebController
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

        $userrole = UserRole::find($reqinfo->id);
        if (empty($userrole)) {
            return $this->error("404", "userrole({$reqinfo->id}) not found");
        }

        GrantData::where('grant_id', $userrole->id)->where('grant_type', 1)->delete();
        $userrole->delete();

        return $this->success();
    }
}
