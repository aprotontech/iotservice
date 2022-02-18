<?php

namespace App\Http\Controllers\Admin;

use App\Models\System;
use Illuminate\Http\Request;
use Illuminate\Pagination\Paginator;

class SystemListController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            "name" => 'optional | is_string',
            "page" => "is_int",
            "size" => "is_int",
        ];
    }

    public function handle(Request $request)
    {
        $reqinfo = $this->reqToObject($request);
        $name = isset($reqinfo->name) ? trim($reqinfo->name) : '';

        $page = intval($reqinfo->page);
        $size = intval($reqinfo->size);

        Paginator::currentPageResolver(function () use ($page) {
            return $page;
        });

        if (!empty($name)) {
            $list = System::where('name', 'like', '%' . $name . '%')
                ->orderBy('id', 'desc')
                ->paginate($size);
        } else {
            $list = System::orderBy('id', 'desc')
                ->paginate($size);
        }

        return $this->formatData($list);
    }

    private function formatData($list)
    {
        $data = ['total' => 0, 'list' => []];

        if ($list) {
            $data['total'] = $list->total();
            foreach ($list as $info) {
                $row = [];
                $row['id'] = $info->id;
                $row['name'] = $info->name;
                $row['desc'] = $info->desc;
                $row['createdAt'] = $info->created_at->toDateTimeString();
                $data['list'][] = $row;
            }
        }

        return $this->success($data);
    }
}
