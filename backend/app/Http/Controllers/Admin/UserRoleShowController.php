<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\User;

class UserRoleShowController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            "userId" => "is_int",
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $user = User::with('userroles', 'userroles.role')->find($reqinfo->userId);
        if (empty($user)) {
            return $this->error("404", "user({$reqinfo->userId}) not found");
        }

        $data = [];
        $data['account'] = $user->email;
        $data['name'] = $user->name;
        $data['id'] = $user->id;
        $data['roles'] = [];

        foreach ($user->userroles as $userrole) {
            $arr = [];
            $arr['id'] = $userrole->id;
            $arr['roleId'] = $userrole->role_id;
            $arr['role'] = $userrole->role->name;
            $arr['system'] = $userrole->role->system;
            $arr['dataRule'] = $userrole->data_rule == '*' ? 'all' : $userrole->data_rule;
            $arr['createdAt'] = $userrole->created_at->toDateTimeString();

            $data['roles'][] = $arr;
        }

        return $this->success($data);
    }
}
