<?php

namespace App\Http\Controllers\Admin;

use App\Models\Role;
use App\Models\Permission;
use Illuminate\Http\Request;
use Illuminate\Pagination\Paginator;

class RoleListController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            "role" => "optional | is_string",
            "sys" => "optional | is_string",
            "type" => 'optional | is_int',
            "page" => "optional | is_int",
            "size" => "optional | is_int",
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $role = isset($reqinfo->role) ? trim($reqinfo->role) : '';
        $system = isset($reqinfo->sys) ? trim($reqinfo->sys) : '';
        $type = isset($reqinfo->type) ? intval($reqinfo->type) : 0;
        $page = isset($reqinfo->page) ? intval($reqinfo->page) : 1;
        $size = isset($reqinfo->size) ? intval($reqinfo->size) : 20;

        Paginator::currentPageResolver(function () use ($page) {
            return $page;
        });
        $list = Role::searchAll($role, $system, $type)
            ->orderBy('id', 'desc')
            ->paginate($size);

        return $this->formatData($list);
    }

    private function formatData($list)
    {
        $data = ['total' => 0, 'list' => []];

        if ($list) {
            $data['total'] = $list->total();
            foreach ($list as $role) {
                $row = [];
                $row['id'] = $role->id;
                $row['name'] = $role->name;
                $row['desc'] = $role->desc;
                $row['system'] = $role->system;
                $row['type'] = $role->type;
                $row['createdAt'] = $role->created_at->toDateTimeString();
                $row['updatedAt'] = $role->updated_at->toDateTimeString();
                $data['list'][] = $row;
            }
        }

        return $this->success($data);
    }
}
