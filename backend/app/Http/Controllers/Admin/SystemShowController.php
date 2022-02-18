<?php

namespace App\Http\Controllers\Admin;

use Illuminate\Http\Request;
use App\Models\System;

class SystemShowController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            "id" => 'is_int',
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $systeinfo = System::find($reqinfo->id);
        if (empty($systeinfo)) {
            return $this->error("404", "system($reqinfo->id) not found");
        }

        return $this->success([
            'id' => $systeinfo->id,
            'name' => $systeinfo->name,
            'desc' => $systeinfo->desc,
            'createdAt' => $systeinfo->created_at->toDateTimeString(),
        ]);
    }
}
