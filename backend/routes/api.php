<?php

use Illuminate\Http\Request;
use Illuminate\Support\Facades\Route;
use App\Http\Controllers\HttpDnsController;
use App\Http\Controllers\DebugerController;

/*
|--------------------------------------------------------------------------
| API Routes
|--------------------------------------------------------------------------
|
| Here is where you can register API routes for your application. These
| routes are loaded by the RouteServiceProvider within a group which
| is assigned the "api" middleware group. Enjoy building your API!
|
*/

Route::get('/version', function (Request $request) {
    return json_encode([
        "version" => env('APP_VERSION', '1.0')
    ]);
});

Route::post('/device/session', 'IotApi\DeviceSession\NewSessionController@route');
Route::post('/device/attrs', 'IotApi\Property\DeviceAttributeController@route');

Route::post('/device/dns', 'IotApi\ServiceDns\HttpDnsController@route');

Route::post('/location/report', 'IotApi\Location\ReportController@route');

Route::post('/time', 'IotApi\TimeServer\ServerTimeController@route');

Route::post('/mqtt/auth', 'EMQ\AuthController@route');
Route::post('/mqtt/acl', 'EMQ\AclController@route');
Route::post('/mqtt/superuser', 'EMQ\SuperUserController@route');
Route::post('/mqtt/webhook', 'EMQ\WebHookController@route');
