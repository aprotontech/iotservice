<?php

namespace App\Jobs;

use DB;
use LRedis as Redis;
use Illuminate\Bus\Queueable;
use Illuminate\Queue\SerializesModels;
use Illuminate\Queue\InteractsWithQueue;
use Illuminate\Contracts\Queue\ShouldQueue;
use Illuminate\Foundation\Bus\Dispatchable;

use App\Device;

class DeviceAlive implements ShouldQueue
{
    use Dispatchable, InteractsWithQueue, Queueable, SerializesModels;

    private $clientId;
    private $active;
    /**
     * Create a new job instance.
     *
     * @return void
     */
    public function __construct($clientId, $active = false)
    {
        $this->clientId = $clientId;
        $this->active = $active;
    }

    /**
     * Execute the job.
     *
     * @return void
     */
    public function handle()
    {
        rclog_notice(__CLASS__ . '::' . __FUNCTION__);
        $aliveKey = '/device/day-alive';

        try {
            $today = date('Y-m-d');
            $redis = Redis::connection();
            $v = $redis->hget($aliveKey, $this->clientId);
            if ($v == $today) return;

            $device = Device::getDevice($this->clientId);
            $r = $device->markActive();
            try {
                DB::table('device_daily_alive_records')
                    ->insert([
                        'app_id' => $device->app_id,
                        'client_id' => $device->client_id,
                        'day' => $today,
                        'ative' => $r || $this->active ? 1:0,
                        'created_at' => rc_datetime(),
                        'updated_at' => rc_datetime(),
                    ]);
            } catch (\Throwable $e) {
                rclog_exception($e, false);
            }

            $redis->hset($aliveKey, $this->clientId, $today);
            $redis->expireAt($aliveKey, strtotime(date('Y-m-d')) + 24 * 3600 - 1);

        } catch (\Exception $e) {
            rclog_exception($e, true);
        }
    }


}
