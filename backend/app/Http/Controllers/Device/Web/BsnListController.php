<?php

namespace App\Http\Controllers\Device\Web;

use DB;
use Illuminate\Http\Request;

class BsnListController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public $permission = 'bsn_view';

    public function getFilter()
    {
        return [
            'page' => 'is_int',
            'size' => 'is_int',
            'query' => 'is_string',
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);

        $sql = DB::table('factory_bsns');

        if ($info->query) {
            //            $sql->where(function ($query) use ($info) {
            $sql->where('bsn', 'like', '%' . $info->query . '%')
                ->orWhere('mac', 'like', '%' . $info->query . '%');
            //            });
        }

        $count = max(1, min($info->size, 200));
        $page = max(1, $info->page);

        $total = $sql->count();

        $list = $sql->skip(($page - 1) * $count)->take($count)
            ->get();

        return $this->success([
            'total' => $total,
            'list' => $list
        ]);
    }
}
