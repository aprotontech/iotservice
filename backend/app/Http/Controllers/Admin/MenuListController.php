<?php

namespace App\Http\Controllers\Admin;

use App\Models\Menu;
use Illuminate\Http\Request;
use Illuminate\Pagination\Paginator;

class MenuListController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            "title" => "is_string",
            "type" => "is_string",
            "sys" => "is_string",
            "page" => "is_int",
            "size" => "is_int",
        ];
    }
    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $title = isset($reqinfo->title) ? trim($reqinfo->title) : '';
        $system = isset($reqinfo->sys) ? trim($reqinfo->sys) : '';
        $page = isset($reqinfo->page) ? intval($reqinfo->page) : 1;
        $size = isset($reqinfo->size) ? intval($reqinfo->size) : 20;
        $type = isset($reqinfo->type) ? trim($reqinfo->type) : '';

        Paginator::currentPageResolver(function () use ($page) {
            return $page;
        });
        $list = Menu::searchAll($title, $system, $type)
            ->orderBy('id', 'desc')
            ->paginate($size);

        return $this->formatData($list);
    }

    private function formatData($list)
    {
        $data = ['total' => 0, 'list' => []];

        if ($list) {
            $data['total'] = $list->total();
            foreach ($list as $menu) {
                $row = [];
                $row['id'] = $menu->id;
                $row['title'] = $menu->title;
                $row['system'] = $menu->system;
                $row['pid'] = $menu->pid;
                $row['url'] = $menu->url;
                $row['type'] = $menu->type;
                $row['sort'] = $menu->sort;
                $row['desc'] = $menu->desc;
                $row['attrs'] = $menu->attrs;

                $row['createdAt'] = $menu->created_at->toDateTimeString();
                $row['updatedAt'] = $menu->updated_at->toDateTimeString();
                $data['list'][] = $row;
            }
        }

        return $this->success($data);
    }
}
