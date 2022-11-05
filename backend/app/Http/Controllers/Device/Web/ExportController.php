<?php

namespace App\Http\Controllers\Device\Web;

use Illuminate\Http\Request;
use Illuminate\Support\Facades\DB;

use Proton\Framework\Media\Storage;

class ExportController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public $permission = 'device_export;appId=app_id';

    public function getFilter()
    {
        return [
            'app_id' => 'is_string',
            'startId' => 'is_string',
            'endId' => 'is_string'
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);

        $query = DB::table('devices');
        if ($info->app_id) {
            $query->where('app_id', $info->app_id);
        }

        if ($info->startId) {
            $query->where('client_id', '>=', strtoupper($info->startId));
        }

        if ($info->endId) {
            $query->where('client_id', '<=', strtoupper($info->endId));
        }

        $list = $query->limit(50000)
            ->select('app_id', 'client_id', 'uuid')
            ->get();

        $clients = [];
        foreach ($list as $m) {
            $clients[$m->client_id] = $m;
            $m->public_key = '';
            $m->secret = '';
        }

        $list = DB::table('device_keys')
            ->whereIn('client_id', array_keys($clients))
            ->get();

        foreach ($list as $m) {
            $c = $clients[$m->client_id];
            $c->public_key = $this->formatKey($m->public_key);
            $c->secret = $m->secret;
        }

        $url = $this->toCSVFileUrl(array_values($clients));
        return $this->success([
            'downloadUrl' => $url
        ]);
    }

    private function formatKey($key)
    {
        $m = [];
        foreach (explode("\n", $key) as $s) {
            $s = trim($s);
            if (!$s) continue;
            if (strpos($s, '---') === false) {
                array_push($m, $s);
            }
        }

        return implode('', $m);
    }

    private function toCSVFileUrl($devices)
    {

        $m = [];
        array_push($m, "clientId,publicKey,secret,uuid");
        foreach ($devices as $dev) {
            array_push($m, "$dev->client_id,$dev->public_key,$dev->secret,$dev->uuid");
        }
        $r = Storage::saveFileContent(implode("\n", $m), "CSV", "csv");
        if ($r->isError()) return '';
        return $r->url;
    }
}
