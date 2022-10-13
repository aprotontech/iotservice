<?php

namespace App\Http\Controllers\Device\SnHelper;

use DB;

class Register
{
    private $appId;

    public function __construct($appId)
    {
        $this->appId = $appId;
    }

    public function registClients($clients, $status, $transaction = true)
    {
        $hint = md5(getmypid() . microtime(true) . __FILE__);

        $devices = [];
        $devkeys = [];
        foreach ($clients as $c) {
            $now = date('Y-m-d H:i:s');
            array_push($devices, [
                'app_id' => $this->appId,
                'uuid' => $c->uuid,
                'client_id' => $c->clientId,
                'status' => $status,
                'created_at' => $now,
                'updated_at' => $now,
                'mac' => $c->mac
            ]);

            array_push($devkeys, [
                'client_id' => $c->clientId,
                'secret' => $c->secret,
                'salt' => substr(md5(rand() . $hint), 8, 16),
                'public_key' => $c->publicKey,
                'private_key' => $c->privateKey,
                'created_at' => $now,
                'updated_at' => $now
            ]);
        }

        try {
            $insertSql = function () use ($devices, $devkeys) {
                DB::table('device_keys')->insert($devkeys);
                DB::table('devices')->insert($devices);
            };
            if ($transaction) {
                DB::transaction($insertSql);
            } else {
                $insertSql();
            }
        } catch (\Exception $e) {
            rclog_exception($e);
            return false;
        }

        return true;
    }
}
