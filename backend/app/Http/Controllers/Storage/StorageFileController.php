<?php

namespace App\Http\Controllers\Storage;

use DB;
use Illuminate\Http\Request;

class StorageFileController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public function handle(Request $request)
    {
        $prefix = env('MEDIA_URL_PREFIX', '');
        $urlPath = '/' . $request->path();
        if (strpos($urlPath, $prefix) === 0) {
            $file = str_replace('//', '/', storage_path('/app/' . substr($urlPath, strlen($prefix))));
            if (file_exists($file)) {
                $m = pathinfo($file);
                rclog_info(json_encode($m));

                return response()->download($file, $m['basename']);
            }
        }

        return response('Not Found File', 404);
    }
}
