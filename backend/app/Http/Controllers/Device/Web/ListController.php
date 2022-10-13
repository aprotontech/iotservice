<?php

namespace App\Http\Controllers\Device\Web;

use DB;
use Illuminate\Http\Request;

class ListController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public $permission = 'device_view';

    public function getFilter()
    {
        return [
            'app_id' => 'optional | is_string',
            'client_id' => 'optional | is_string',
            'uuid' => 'optional | is_string',
            'status' => 'optional | is_int',
            'is_online' => 'optional | is_int',
            'show_not_assign' => 'optional | is_bool',
            'page' => 'optional | is_numeric',
            'size' => 'optional | is_numeric',
            'draw' => 'optional | is_numeric',
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);
        $list = $this->getList($request, (array)$info);

        return $this->success($list);
    }

    private function getList($request, $filter)
    {
        $query = DB::table('devices')
            ->where('status', '>=', $filter['show_not_assign'] ? 0 : 1)
            ->leftJoin('device_locations', 'devices.client_id', '=', 'device_locations.client_id');
  
        $queryCnt = DB::table('devices')
            ->where('status', '>=', $filter['show_not_assign'] ? 0 : 1);

        if (!empty($filter['app_id'])) {
            $arr = [];
            foreach (explode(',', $filter['app_id']) as $m) {
                $m = strtolower(trim($m));
                if ($m) array_push($arr, $m);
            }

            $appList = $this->getAppList($request, $arr);
            $arr = array_keys($appList);
            if ($arr) {
                $query->whereIn('app_id', $arr);
                $queryCnt->whereIn('app_id', $arr);
            }
        } else {
            $appList = $this->getAppList($request);
            $arr = array_keys($appList);
            if ($arr) {
                $query->whereIn('app_id', $arr);
                $queryCnt->whereIn('app_id', $arr);
            }
        }

        if (!empty($filter['client_id'])) {
            $query->where('devices.client_id', 'like', '%' . $filter['client_id'] . '%');
            $queryCnt->where('client_id', 'like', '%' . $filter['client_id'] . '%');
        }

        if (!empty($filter['uuid'])) {
            $arr = [];
            foreach (explode(',', $filter['uuid']) as $m) {
                $m = trim($m);
                if ($m) array_push($arr, $m);
            }
            if ($arr) {
                $query->whereIn('uuid', $arr);
                $queryCnt->whereIn('uuid', $arr);
            }
        }

        if (isset($filter['status'])) {
            $query->where('status', $filter['status']);
            $queryCnt->where('status', $filter['status']);
        }

        if (isset($filter['is_online'])) {
            switch ($filter['is_online']) {
                case 1:
                    $query->where('is_online', 1);
                    $queryCnt->where('is_online', 1);
                    break;
                case 2:
                    $query->where('is_online', 0);
                    $queryCnt->where('is_online', 0);
                    break;
            }
        }

        if (isset($filter['online_stime']) && $filter['online_stime']) {
            $query->where('online_time', '>=', $filter['online_stime']);
            $queryCnt->where('online_time', '>=', $filter['online_stime']);
        }

        if (isset($filter['online_etime']) && $filter['online_etime']) {
            $query->where('online_time', '<=', $filter['online_etime']);
            $queryCnt->where('online_time', '<=', $filter['online_etime']);
        }

        if (isset($filter['offline_stime']) && $filter['offline_stime']) {
            $query->where('offline_time', '>=', $filter['offline_stime']);
            $queryCnt->where('offline_time', '>=', $filter['offline_stime']);
        }

        if (isset($filter['offline_etime']) && $filter['offline_etime']) {
            $query->where('offline_time', '<=', $filter['offline_etime']);
            $queryCnt->where('offline_time', '<=', $filter['offline_etime']);
        }

        if (isset($filter['active_stime']) && $filter['active_stime']) {
            $query->where('active_time', '>=', $filter['active_stime']);
            $queryCnt->where('active_time', '>=', $filter['active_stime']);
        }
        if (isset($filter['active_etime']) && $filter['active_etime']) {
            $query->where('active_time', '<=', $filter['active_etime']);
            $queryCnt->where('active_time', '<=', $filter['active_etime']);
        }

        if (isset($filter['is_active'])) {
            switch ($filter['is_active']) {
                case 1:
                    $query->where('active_time', '>', 0);
                    $queryCnt->where('active_time', '>', 0);
                    break;
                case 2:
                    $query->whereNull('active_time');
                    $queryCnt->whereNull('active_time');
                    break;
            }
        }

        if (isset($filter['city'])) {
            $arr = [];
            foreach (explode(',', $filter['city']) as $m) {
                $m = trim($m);
                if ($m) array_push($arr, $m);
            }

            if ($arr) $query->whereIn('city', $arr);
        }

        // todo 设置city的情况下，count可能会不匹配
        $total = $queryCnt->count();

        $page = isset($filter['page']) ? $filter['page'] : 1;
        $size = isset($filter['size']) ? $filter['size'] : 20;
        $skip = ($page - 1) * $size;
        $query->skip($skip)->take($size);

        $list = $query->select(DB::raw(
            "devices.id,devices.client_id,uuid,app_id,is_online,active_time,city," .
                "active_time,status,deleted_at,offline_time,online_time"
        ))->get();

        return [
            'total' => $total,
            'page' => $page,
            'size' => $size,
            'list' => $list,
        ];
    }
}
