<?php

namespace App\Http\Controllers\Device\SnHelper;

class TaskStatus
{
    const TASK_CREATED = 0;
    const TASK_ACCEPTED = 1;
    const TASK_DENY = 2;
    const TASK_PROCESSING = 3;
    const TASK_FAILED = 4;
    const TASK_DONE = 5;

    public static function statusToString($status)
    {
        $m = [
            self::TASK_CREATED => '审核中',
            self::TASK_ACCEPTED => '审核通过',
            self::TASK_DENY => '审核拒绝',
            self::TASK_PROCESSING => '正在生产',
            self::TASK_FAILED => '生成失败',
            self::TASK_DONE => '完成'
        ];

        return $m[$status];
    }
}
