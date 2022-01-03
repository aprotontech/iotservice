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

Route::middleware('auth:sanctum')->get('/user', function (Request $request) {
    return $request->user();
});


Route::get('/version', function (Request $request) {
    return json_encode([
        "version" => env('APP_VERSION', '1.0')
    ]);
});

Route::resource('/dns', 'HttpDnsController');
Route::resource('/devices', 'DeviceController');
Route::resource('/device/attrs', 'DeviceAttributeController');

Route::get('/debug/session', [DebugerController::class, 'getSession']);
Route::get('/debug/device', [DebugerController::class, 'getDevice']);