<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\User;
use App\Models\Role;
use App\Models\UserRole;
use App\Models\GrantData;

class UserRoleAddController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'userId' => "is_int",
            'roleId' => 'is_int',
            'sys' => 'is_string',
            'dataRule' => 'optional | is_string',
            'data' => 'optional | is_array',
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $user = User::find($reqinfo->userId);
        $role = Role::find($reqinfo->roleId);

        if (empty($user)) {
            return $this->error("404", "user({$reqinfo->userId}) not found");
        }

        if (empty($role)) {
            return $this->error("404", "role({$reqinfo->roleId}) not found");
        }

        return $this->save($reqinfo);
    }

    private function save($reqinfo)
    {
        $dataRule = isset($reqinfo->dataRule) ? trim($reqinfo->dataRule) : 'all';
        $userId = intval($reqinfo->userId);
        $roleId = intval($reqinfo->roleId);
        $system = trim($reqinfo->sys);
        $data = isset($reqinfo->data) ? $reqinfo->data : [];

        $userrole = UserRole::where('user_id', $userId)->where('role_id', $roleId)->first();
        if (empty($userrole)) {
            $userrole = new UserRole();
        }
        $userrole->user_id = $userId;
        $userrole->role_id = $roleId;
        $userrole->data_rule = $dataRule;
        $userrole->system = $system;

        $userrole->save();

        if ($data) {
            $grantData = [];
            foreach ($data as $row) {
                $grantData[] = [
                    'grant_type' => 1,
                    'grant_id' => $userrole->id,
                    'data' => strval($row),
                    'created_at' => date('Y-m-d H:i:s'),
                    'updated_at' => date('Y-m-d H:i:s'),
                ];
            }
            GrantData::where('grant_id', $userrole->id)->where('grant_type', 1)->delete();
            GrantData::insert($grantData);
        } else {
            GrantData::where('grant_id', $userrole->id)->where('grant_type', 1)->delete();
        }

        return $this->success();
    }
}
