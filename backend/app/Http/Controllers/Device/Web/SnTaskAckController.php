<?php

namespace App\Http\Controllers\Device\Web;

use DB;
use App\Jobs\SnTask;
use Illuminate\Http\Request;

use App\Http\Controllers\Device\SnHelper\TaskStatus;

class SnTaskAckController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public $permission = 'sn_task_ack';

    public function getFilter()
    {
        return [
            'task_id' => 'is_numeric',
            'status' => 'is_int'
        ];
    }

    public function route(Request $request)
    {
        $info = $this->reqToObject($request);
        if (!in_array($info->status, [TaskStatus::TASK_ACCEPTED, TaskStatus::TASK_DENY])) {
            return $this->error("100", "input status must be 1 or 2");
        }

        try {
            $r = DB::table('sn_tasks')
                ->where('id', $info->task_id)
                ->where('status', TaskStatus::TASK_CREATED)
                ->update([
                    'status' => $info->status,
                    'acker' => $request->user()->name,
                    'acked_at' => date('Y-m-d H:i:s'),
                ]);
            if ($info->status == TaskStatus::TASK_ACCEPTED && $r) {
                SnTask::dispatch([
                    'taskId' => $info->task_id
                ]);
                rclog_notice("add new sntask");
            }
        } catch (\Exception $e) {
            rclog_exception($e, true);
            $msg = $e->getMessage();
            return $this->error("500", "update task($info->task_id) to status($info->status) failed with($msg)");
        }

        return $this->success();
    }
}
