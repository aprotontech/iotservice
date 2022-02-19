<?php

namespace App\Jobs;

use DB;
use LRedis as Redis;
use Illuminate\Bus\Queueable;
use Illuminate\Queue\SerializesModels;
use Illuminate\Queue\InteractsWithQueue;
use Illuminate\Contracts\Queue\ShouldQueue;
use Illuminate\Foundation\Bus\Dispatchable;

use App\Models\Device;

class DeviceAlive implements ShouldQueue
{
    use Dispatchable, InteractsWithQueue, Queueable, SerializesModels;

    private $clientId;
    private $online;
    /**
     * Create a new job instance.
     *
     * @return void
     */
    public function __construct($clientId, $online = false)
    {
        $this->clientId = $clientId;
        $this->online = $online;
    }

    /**
     * Execute the job.
     *
     * @return void
     */
    public function handle()
    {
        rclog_notice(__CLASS__ . '::' . __FUNCTION__);

        $device = Device::where('client_id', $this->clientId)->first();
        if ($device) {
            $this->updateStatus($device);
        }
    }

    private function updateStatus($device)
    {
        $now = date("Y-m-d H:i:s", time());
        if ($this->online) {
            $device->is_online = 1;
            $device->online_time = $now;

            if (!$device->active_time) {
                $device->active_time = $now;
            }
        } else {
            $device->is_online = 0;
            $device->offline_time = $now;
        }

        $device->save();
    }

    private function recordActive($device)
    {
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
                        'ative' => $r || $this->active ? 1 : 0,
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
