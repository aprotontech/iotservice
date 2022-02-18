<?php

namespace App\Http\Controllers\Device\Web;

use DB;
use Illuminate\Http\Request;

use App\Jobs\SnTask;

class TaskQueueNotifyController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public $permission = 'sn_task';

    public function getFilter()
    {
        return [];
    }

    public function handle(Request $request)
    {
        SnTask::dispatch([
            'taskId' => 0
        ]);

        return $this->success();
    }
}
