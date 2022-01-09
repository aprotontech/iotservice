<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\User;
use App\Models\Role;
use App\Models\UserRole;
use App\Models\GrantData;

class UserRoleEditController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'id' => 'is_int',
            'dataRule' => 'is_string',
            'data' => 'optional | is_array',
        ];
    }
    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $userrole = UserRole::find($reqinfo->id);
        if (empty($userrole)) {
            return $this->error("404", "userrole({$reqinfo->id}) not found");
        }
        $userrole->data_rule = $reqinfo->dataRule;
        $userrole->save();

        GrantData::where('grant_id', $userrole->id)->where('grant_type', 1)->delete();
        if (isset($reqinfo->data)) {
            $grantData = [];
            foreach ($reqinfo->data as $row) {
                $grantData[] = [
                    'grant_type' => 1,
                    'grant_id' => $userrole->id,
                    'data' => strval($row),
                    'created_at' => date('Y-m-d H:i:s'),
                    'updated_at' => date('Y-m-d H:i:s'),
                ];
            }
            GrantData::insert($grantData);
        }

        return $this->success();
    }
}
