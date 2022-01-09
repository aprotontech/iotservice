<?php

namespace App\Http\Controllers\Device\Web;

use Illuminate\Http\Request;
use DB;


class TokenController extends \App\Http\Controllers\WebController
{
    const ONE_HOUR = 60 * 60; //3600秒

    use \Proton\Framework\ApiControllerTrait;

    public $permission = 'device_view';

    public function getFilter()
    {
        return [
            'client_id' => 'is_string',
        ];
    }

    public function handle(Request $request)
    {
        $log_prefix = "[TokenController-handle]";
        $info = $this->reqToObject($request);

        $record = DB::table('devices')
            ->where('client_id', $info->client_id)
            ->first();

        if (!$record) {
            return $this->error(500, "client_id(" . $info->client_id . ") query token fail.");
        }

        $token = $record->session;
        $token_timeout = $record->session_timeout;
        if (!$token || !$token_timeout) {
            return $this->error(500, "client_id(" . $info->client_id . ") tokeninfo is invalid. token=" . $token . " token_timeout=" . $token_timeout);
        }

        $token_valid_sec = strtotime($token_timeout);
        $now_sec = time();
        //若token过期，则更新过期时间
        if ($now_sec > $token_valid_sec) {
            $new_timeout = $now_sec + 6 * TokenController::ONE_HOUR;
            $token_timeout = date("Y-m-d H:i:s", $new_timeout);
            $sql = "update devices set session_timeout = '" . $token_timeout . "' where id=?";
            $affected = DB::update($sql, [$record->id]);
            rclog_debug($log_prefix . " client_id = " . $info->client_id . " id = " . $record->id . " affected = " . $affected . " new_timeout=" . $token_timeout);
        }

        //        rclog_debug("[test] token_timeout = " . $token_timeout . " token_valid_ms = " . $token_valid_ms . " now_ms = " . $now_ms);
        return $this->success([
            'token' => $token,
            'token_timeout' => $token_timeout
        ]);
    }
}
