<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\User;
use App\Models\UserRole;

class UserRoleInfoController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            "id" => "is_int",
        ];
    }
    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $role = UserRole::find($reqinfo->id);
        if (empty($role)) {
            return $this->error("404", "userrole({$reqinfo->id}) not found");
        }

        $data = [];
        $data['id'] = $role->id;
        $data['user_id'] = $role->user_id;
        $data['role_id'] = $role->role_id;
        $data['role_name'] = $role->role->name;
        $data['account'] = $role->user->email;
        $data['user_name'] = $role->user->name;
        $data['data_rule'] = $role->data_rule;
        $data['system'] = $role->system;
        $data['datas'] = '';
        if ($role->grantdatas) {
            foreach ($role->grantdatas as $d) {
                $data['datas'] .= $d->data . "\n";
            }
        }

        $data['createdAt'] = $role->created_at->toDateTimeString();

        return $this->success($data);
    }
}
