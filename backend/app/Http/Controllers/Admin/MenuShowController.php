<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\Menu;
use App\Models\RoleMenu;


class MenuShowController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'id' => 'is_int',
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $m = Menu::find($reqinfo->id);
        if (empty($m)) {
            return $this->error("404", "invalidate menu");
        }

        return $this->success($m->toArray());
    }
}
