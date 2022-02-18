<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\Menu;

class MenuAddController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            "sessionId" => "optional | is_string",
            'title' => 'optional | is_string',
            'url' => 'optional | is_string',
            'pid' => 'optional | is_int',
            'type' => 'optional | is_string',
            'sort' => 'optional | is_int',
            'sys' => 'optional | is_string',
            'desc' => 'optional | is_string',
            'attrs' => 'optional | is_string',
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $m = new Menu();
        $m->title = trim($reqinfo->title);
        $m->url = isset($reqinfo->url) ? trim($reqinfo->url) : '';
        $m->pid = isset($reqinfo->pid) ? intval($reqinfo->pid) : 0;
        $m->type = isset($reqinfo->type) ? trim($reqinfo->type) : '';
        $m->sort = isset($reqinfo->sort) ? intval($reqinfo->sort) : 0;
        $m->system = isset($reqinfo->sys) ? trim($reqinfo->sys) : '';
        $m->desc = isset($reqinfo->desc) ? trim($reqinfo->desc) : '';
        $m->attrs = isset($reqinfo->attrs) ? trim($reqinfo->attrs) : '';

        $m->save();
        return $this->success();
    }
}
