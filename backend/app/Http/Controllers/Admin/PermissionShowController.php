<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\Permission;

class PermissionShowController extends \App\Http\Controllers\WebController
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

        return $this->formatData($p);
    }

    private function formatData($p)
    {
        $data = [];

        $data['id'] = $p->id;
        $data['name'] = $p->name;
        $data['permission'] = $p->permission;
        $data['description'] = $p->description;
        $data['system'] = $p->system;
        $data['createdAt'] = $p->created_at->toDateTimeString();
        $data['updatedAt'] = $p->updated_at->toDateTimeString();

        return $this->success($data);
    }
}
