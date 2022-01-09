<?php

namespace App\Http\Controllers\Device\Web;

use Illuminate\Http\Request;

use DB;

class BsnUnbindController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public $permission = 'bsn_bind';

    public function getFilter()
    {
        return [
            'bsnList' => is_array_of([
                'mac' => 'is_string',
                'bsn' => 'is_string & is_not_empty'
            ])
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);

        foreach ($info->bsnList as $t) {
            DB::table('factory_bsns')
                ->where('bsn', $t->bsn)
                ->where('mac', $t->mac)
                ->update([
                    'mac' => null,
                    'binder' => '',
                    'updated_at' => null
                ]);
        }

        return $this->success();
    }
}
