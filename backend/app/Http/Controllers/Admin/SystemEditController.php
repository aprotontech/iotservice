<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\System;

class SystemEditController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'id' => 'is_int',
            'desc' => 'is_string',
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);
        $info = System::find($reqinfo->id);
        if (empty($info)) {
            return $this->error("404", "id({$reqinfo->id}) not found");
        }

        $info->desc = trim($reqinfo->desc);
        $info->save();

        return $this->success();
    }
}
