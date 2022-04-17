<?php

namespace App\Http\Controllers\IotApi;

use Illuminate\Support\Facades\DB;
use Proton\Framework\Exception\ApiException;

use App\Models\Device;

class DeviceUtils
{

    const DEVICE_SESSION_TIMEOUT = 24 * 3600;

    public static function getDeviceByClientId($clientId)
    {
        $device = Device::where('client_id', $clientId)
            ->first();

        if (!$device) {
            throw new ApiException(404, "not found device of clientId($clientId)");
        }

        return $device;
    }

    public static function getClientIdByUUID($appId, $UUID)
    {
        $device = Device::where('app_id', $appId)
            ->where('uuid', $UUID)
            ->first();

        if ($device) return $device->client_id;
        return false;
    }

    public static function newSession(Device $device)
    {
        $device->session = md5(md5(microtime(true) . $device->deviceId . $device->signature . rand(0, 10000)) . $device->appId);
        $device->session_timeout = rc_datetime(time() + self::DEVICE_SESSION_TIMEOUT);
        $device->save();

        return $device;
    }

    public static function regist($appId, $uuid)
    {
        if (empty($appId) || empty($uuid)) {
            return null;
        }
        DB::table('devices')
            ->where('app_id', $appId)
            ->whereRaw('uuid=client_id')
            ->where('status', 0)
            ->limit(1)
            ->update([
                'uuid' => $uuid,
                'status' => 1,
            ]);

        $device = Device::where('app_id', $appId)
            ->where('uuid', $uuid)
            ->first();
        return $device;
    }
}
