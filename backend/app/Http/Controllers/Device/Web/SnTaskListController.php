<?php

namespace App\Http\Controllers\Device\Web;

use DB;
use Illuminate\Http\Request;
use App\Http\Controllers\Device\Web\SnHelper\TaskStatus;

class SnTaskListController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public $permission = 'sn_task';

    public function getFilter()
    {
        return [
            'app_id' => is_array_of('is_string'),
            'page' => 'is_int',
            'size' => 'is_int',
            'query' => 'is_string'
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);

        $appList = $this->getAppList($request, empty($info->app_id) || empty($info->app_id[0]) ? [] : $info->app_id);
        rclog_debug(json_encode($appList));

        $sql = DB::table('sn_tasks')
            ->whereIn('app_id', array_keys($appList));

        if ($info->query) {
            $sql->where('name', '%' . $info->query . '%');
        }

        $count = max(1, min($info->size, 200));
        $page = max(1, $info->page);

        $total = $sql->count();

        $list = $sql->skip(($page - 1) * $count)->take($count)
            ->get();

        foreach ($list as $t) {
            $t->statusString = TaskStatus::statusToString($t->status);
        }

        return $this->success([
            'total' => $total,
            'list' => $list
        ]);
    }
}
