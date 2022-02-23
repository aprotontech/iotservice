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

/*
Route::get('/debug/session', [DebugerController::class, 'getSession']);
Route::get('/debug/device', [DebugerController::class, 'getDevice']);
*/

Route::post('/device/session', 'IotApi\DeviceSession\NewSessionController@route');
Route::post('/device/attrs', 'IotApi\Property\DeviceAttributeController@route');

Route::post('/device/dns', 'IotApi\ServiceDns\HttpDnsController@route');

Route::post('/time', 'IotApi\TimeServer\ServerTimeController@route');

Route::post('/mqtt/auth', 'Mqtt\AuthController@route');
Route::post('/mqtt/acl', 'Mqtt\AclController@route');
Route::post('/mqtt/superuser', 'Mqtt\SuperUserController@route');
Route::post('/mqtt/webhook', 'Mqtt\WebHookController@route');
