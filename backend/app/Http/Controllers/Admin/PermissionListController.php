<?php

namespace App\Http\Controllers\Admin;

use App\Models\Permission;
use Illuminate\Http\Request;
use Illuminate\Pagination\Paginator;

class PermissionListController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            "permission" => "optional | is_string",
            "sys" => "optional | is_string",
            "page" => "optional | is_int",
            "size" => "optional | is_int",
        ];
    }
    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $permission = isset($reqinfo->permission) ? trim($reqinfo->permission) : '';
        $system = isset($reqinfo->sys) ? trim($reqinfo->sys) : '';
        $page = isset($reqinfo->page) ? intval($reqinfo->page) : 1;
        $size = isset($reqinfo->size) ? intval($reqinfo->size) : 20;

        Paginator::currentPageResolver(function () use ($page) {
            return $page;
        });

        $list = Permission::searchAll($permission, $system)
            ->orderBy('id', 'desc')
            ->paginate($size);
        return $this->formatData($list);
    }

    private function formatData($list)
    {
        $data = ['total' => 0, 'list' => []];

        if ($list) {
            $data['total'] = $list->total();
            foreach ($list as $permission) {
                $row = [];
                $row['id'] = $permission->id;
                $row['name'] = $permission->name;
                $row['permission'] = $permission->permission;
                $row['description'] = $permission->description;
                $row['system'] = $permission->system;
                $row['createdAt'] = $permission->created_at->toDateTimeString();
                $row['updatedAt'] = $permission->updated_at->toDateTimeString();
                $data['list'][] = $row;
            }
        }

        return $this->success($data);
    }
}
