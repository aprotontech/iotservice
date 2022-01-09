<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\Role;

class RoleShowController extends \App\Http\Controllers\WebController
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

        return $this->formatData($role);
    }

    private function formatData($info)
    {
        $data = [];

        $data['id'] = $info->id;
        $data['name'] = $info->name;
        $data['desc'] = $info->desc;
        $data['system'] = $info->system;
        $data['type'] = $info->type;
        $data['permissions'] = [];

        if ($info->rolepermissions) {
            foreach ($info->rolepermissions as $row) {
                $data['permissions'][] = [
                    'id' => $row->id,
                    'permission' => $row->permission,
                    'name' => $row->name,
                    'system' => $row->system,
                ];
            }
        }

        $data['createdAt'] = $info->created_at->toDateTimeString();
        $data['updatedAt'] = $info->updated_at->toDateTimeString();

        return $this->success($data);
    }
}
