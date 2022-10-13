<?php

namespace App\Http\Controllers\Device\Web;

use DB;
use Illuminate\Http\Request;
use App\Http\Controllers\Device\SnHelper\TaskStatus;

class SnTaskCreateController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public $permission = 'sn_task;appId=app_id';

    public function getFilter()
    {
        return [
            'app_id' => 'is_string & is_not_empty',
            'name' => 'is_string & is_not_empty',
            'count' => 'is_int',
            'prefix' => 'is_string & is_not_empty',
            'start' => 'is_string & is_not_empty',
            'comment' => 'is_string',
            'type' => 'is_int',
            'assign_mac' => is_optional_of('is_int'),
        ];
    }

    private function checkSnRepeat($start, $end)
    {
        $r = DB::table('devices')->where('client_id', '>=', $start)
            ->where('client_id', '<=', $end)->first();
        if ($r) {
            return true;
        } else {
            return false;
        }
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);

        if ($info->count <= 0) {
            return $this->error("10", "count($info->count) must max than 0");
        } else if ($info->count >= 100000) {
            return $this->error("10", "count($info->count) must less than 100000");
        } else if (strlen($info->prefix) >= 13) {
            return $this->error("10", "prefix($info->prefix) length must < 13");
        } else if (strlen($info->prefix) < 8) {
            return $this->error("10", "prefix($info->prefix) length must >= 8");
        } else if (strlen($info->start) != 14) {
            return $this->error("10", "start($info->start) sn length is not match 14");
        } else if (strpos($info->start, $info->prefix) !== 0) {
            return $this->error("10", "start($info->start) not match prefix($info->prefix)");
        } else if (!in_array($info->type, [0, 1])) {
            return $this->error("10", "type($info->type) must be 0 or 1");
        } else if (!preg_match('/^[A-F]\d{4}[A-F]\d[A-F]\d*$/', $info->prefix)) {
            return $this->error("10", "input prefix($info->prefix) format is not right");
        }

        $c = strlen($info->start) - strlen($info->prefix);
        $s = intval(substr($info->start, strlen($info->prefix)));
        $end = sprintf("$info->prefix%0{$c}d", $s + $info->count - 1);

        if ($this->checkSnRepeat($info->start, $end)) {
            return $this->error("11", "SN is Repeat start=" . $info->start . " end=" . $end);
        }

        try {
            DB::table('sn_tasks')
                ->insert([
                    'app_id' => $info->app_id,
                    'name' => $info->name,
                    'prefix' => $info->prefix,
                    'start' => $info->start,
                    'end' => $end,
                    'count' => $info->count,
                    'status' => TaskStatus::TASK_CREATED,
                    'type' => $info->type,
                    'creator' => $request->user()->name,
                    'description' => $info->comment,
                    'created_at' => date('Y-m-d H:i:s'),
                    'updated_at' => date('Y-m-d H:i:s'),
                    'assign_mac' => empty($info->assign_mac) ? 0 : 1
                ]);
        } catch (\Exception $e) {
            rclog_exception($e, true);
            $msg = $e->getMessage();
            return $this->error("500", "create task failed with($msg)");
        }

        return $this->success();
    }
}
