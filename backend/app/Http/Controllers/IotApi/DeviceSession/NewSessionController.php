<?php

namespace App\Http\Controllers\IotApi\DeviceSession;

use Illuminate\Http\Request;
use Illuminate\Support\Facades\DB;

use App\Models\Device;
use App\Http\Controllers\IotApi\DeviceUtils;

class NewSessionController extends \App\Http\Controllers\Controller
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'appId' => 'optional | is_string & is_not_empty',
            'deviceId' => 'optional | is_string',
            'clientId' => 'optional | is_string',
            'hardwareInfo' => is_optional_of([
                'cpu' => 'optional | is_string',
                'mac' => 'optional | is_string',
            ]),
            'signature' => 'is_string & is_not_empty',
            'time' => 'is_string'
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);

        $device = false;
        $regist = false;
        if (!empty($info->clientId)) { // input clientId
            $device = DeviceUtils::getDeviceByClientId($info->clientId);
        } else if (!empty($info->deviceId) && isset($info->appId)) {
            $clientId = DeviceUtils::getClientIdByUUID($info->appId, $info->deviceId);
            if (!$clientId) {
                $r = $this->autoRegistClientId($info);
                if ($r->isError()) return $r;
                $device = $r->device;
                $regist = true;
            } else $device = DeviceUtils::getDeviceByClientId($clientId);
        } else if (!empty($info->deviceId) && preg_match('/^[0-9A-F]{14}$/', $info->deviceId) && empty($info->appId)) {
            $device = DeviceUtils::getDeviceByClientId($info->deviceId);
        } else {
            return $this->error(100, "input params must has uuid or clientId, but both is empty");
        }

        if (!$device) {
            return $this->error(1404, "not found the device");
        }

        if ($device->status != 1) {
            return $this->error(403, "device status invalid");
        }

        if (isset($info->appId) && $device->app_id != $info->appId) {
            return $this->error(100, "input appId($info->appId) not match device's real appId($device->app_id)");
        }

        if (!empty($info->deviceId) && $device->uuid != $info->deviceId) {
            return $this->error(1406, "input deviceId($info->deviceId) not match with service($device->uuid)");
        }

        if (!$regist) {
            $hinfo = $this->getHardwareInfo($info);
            $random = isset($info->deviceId) ? $info->deviceId : "";
            if ($hinfo) $random .= ',' . $hinfo;
            $r = DeviceSignature::auth($device, $info->signature, $random, $info->time);
            if ($r->isError()) return $r;
        }

        // alloc new session
        DeviceUtils::newSession($device);

        \App\Jobs\DeviceAlive::dispatch($device->client_id, true);

        return $this->success()
            ->set('appId', $device->app_id)
            ->set('clientId', $device->client_id)
            ->set('session', $device->session)
            ->set('expire', strtotime($device->session_timeout))
            ->set('timeout', DeviceUtils::DEVICE_SESSION_TIMEOUT);
    }

    private function autoRegistClientId($info)
    { // make sure appId secret OK
        $hinfo = $this->getHardwareInfo($info);
        $random = $info->deviceId;
        if ($hinfo) $random .= ',' . $hinfo;
        $r = DeviceSignature::appAuth($info->appId, $random, $info->signature, $info->time);
        if ($r->isError()) return $r;

        $device = DeviceUtils::regist($info->appId, $info->deviceId);
        if (!$device) {
            $m = DB::table('devices')
                ->where('app_id', $info->appId)
                ->where('status', 0)
                ->first();
            if (!$m) {
                return rc_error(1401, "no more device to be alloc for deviceId($info->deviceId), appId($info->appId)");
            }

            return rc_error(1402, "regist device failed for deviceId($info->deviceId), appId($info->appId)");
        }

        return rc_success([
            'device' => $device
        ]);
    }

    private function getHardwareInfo($info)
    {
        if (empty($info->hardwareInfo)) return '';
        $mInfo = json_decode(json_encode($info->hardwareInfo), true);
        ksort($mInfo);

        $tmp = [];
        foreach ($mInfo as $k => $v) {
            array_push($tmp, "$k=$v");
        }

        return implode(",", $tmp);
    }
}
