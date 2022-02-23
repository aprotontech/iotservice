<?php

namespace App\Jobs;

use Illuminate\Support\Facades\DB;
use Illuminate\Bus\Queueable;
use Illuminate\Queue\SerializesModels;
use Illuminate\Queue\InteractsWithQueue;
use Illuminate\Contracts\Queue\ShouldQueue;
use Illuminate\Foundation\Bus\Dispatchable;

use App\Http\Controllers\Device\Web\SnHelper\Creator;
use App\Http\Controllers\Device\Web\SnHelper\Register;
use App\Http\Controllers\Device\Web\SnHelper\TaskStatus;

class SnTask implements ShouldQueue
{
    use Dispatchable, InteractsWithQueue, Queueable, SerializesModels;

    /**
     * Create a new job instance.
     *
     * @return void
     */
    public function __construct()
    {
        //
    }

    /**
     * Execute the job.
     *
     * @return void
     */
    public function handle()
    {
        rclog_notice(__CLASS__ . '::' . __FUNCTION__);
        $task = $this->getNextTask();
        if (!$task) {
            rclog_debug("not found any task");
            return;
        }

        try {
            DB::transaction(function () use ($task) {
                $this->createClients($task);
                $this->markTaskStatus($task, TaskStatus::TASK_DONE);
            });
        } catch (\Exception $e) {

            rclog_exception($e, true);
            $this->markTaskStatus($task, TaskStatus::TASK_FAILED);
        }
    }

    private function markTaskStatus($task, $status)
    {
        DB::table('sn_tasks')
            ->where('id', $task->id)
            ->update([
                'status' => $status,
                'updated_at' => date('Y-m-d H:i:s')
            ]);
    }

    private function createClients($task)
    {
        $client = DB::table('devices')
            ->where('client_id', '>=', $task->start)
            ->where('client_id', '<', $task->end)
            ->first();
        if ($client) {
            throw new \Exception("found exists clientId($client->client_id), task($task->name) failed");
        }

        $batch = 1000;
        $start = intval(substr($task->start, strlen($task->prefix)));
        $total = $task->count;
        $is_assign_mac = $task->assign_mac;

        $creator = new Creator(14, false);
        while ($total > 0) {
            $count = min($total, $batch);
            $total = $total - $batch;

            rclog_info("[SnTask] prefix($task->prefix), start($start), count($count)");
            $clients = $creator->createClients($task->prefix, $start, $count, $is_assign_mac);

            if (!$clients) {
                throw new \Exception("create client failed. no client created.");
            }

            $register = new Register($task->app_id);

            if ($register->registClients($clients, $task->type, false)) {
                rclog_info(sprintf("success created %d clients", count($clients)));
            } else {
                throw new \Exception("save clients to db failed");
            }

            $start += $batch;
        }
    }

    private function getNextTask()
    {
        return DB::transaction(function () {
            $task = DB::table('sn_tasks')
                ->where('status', TaskStatus::TASK_ACCEPTED)
                ->first();
            if ($task) {
                $r = DB::table('sn_tasks')
                    ->where('id', $task->id)
                    ->where('status', TaskStatus::TASK_ACCEPTED)
                    ->update([
                        'status' => TaskStatus::TASK_PROCESSING,
                        'updated_at' => date('Y-m-d H:i:s')
                    ]);
                if ($r) return $task;
            }

            return null;
        });
    }
}
