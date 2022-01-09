<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\System;

class SystemAddController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'name' => 'is_string',
            'desc' => 'is_string',
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);
        $sys = new System();
        $sys->name = trim($reqinfo->name);
        $sys->desc = trim($reqinfo->desc);

        $exists = System::where('name', $sys->name)->exists();
        if ($exists) {
            return $this->error("300", "name is exists");
        }

        $sys->save();
        return $this->success();
    }
}
