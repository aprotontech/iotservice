<?php

use Illuminate\Support\Facades\Route;

/*
|--------------------------------------------------------------------------
| Web Routes
|--------------------------------------------------------------------------
|
| Here is where you can register web routes for your application. These
| routes are loaded by the RouteServiceProvider within a group which
| contains the "web" middleware group. Now create something great!
|
*/

Route::get('/', function () {
    return view('welcome');
});

// get file content
Route::get('/storage/{topic}/{day}/{file}', 'Storage\StorageFileController@route');

Route::get('/debug/login', 'Passport\\DebugUserLoginController@login');

Route::get('/passport/session', 'Passport\\SessionController@getInfo');
Route::post('/passport/menus', 'Passport\\MenuController@route');
Route::post('/user/edit', 'WebApi\\UpdateUserInfoController@route');

Route::post('/logout', 'Passport\\SessionController@logout');
Route::post('/login', 'Passport\\LoginController@route');

Route::get('/device/apps', 'Application\\ApplicationController@route');

Route::post('/apps/list', 'Application\\AppListController@route');
Route::post('/apps/create', 'WebApi\\App\\CreateAppController@route');

Route::post('/admin/user/list', 'Admin\\UserListController@route');
Route::post('/admin/user/add', 'Admin\\CreateUserController@route');
Route::post('/admin/user/reset-passport', 'Admin\\ResetUserPasswordController@route');

Route::post('/admin/system/list', 'Admin\\SystemListController@route');
Route::post('/admin/system/edit', 'Admin\\SystemEditController@route');
Route::post('/admin/system/add', 'Admin\\SystemAddController@route');
Route::post('/admin/system/show', 'Admin\\SystemShowController@route');

Route::post('/admin/permission/list', 'Admin\\PermissionListController@route');
Route::post('/admin/permission/edit', 'Admin\\PermissionEditController@route');
Route::post('/admin/permission/add', 'Admin\\PermissionAddController@route');
Route::post('/admin/permission/del', 'Admin\\PermissionDelController@route');
Route::post('/admin/permission/show', 'Admin\\PermissionShowController@route');

Route::post('/admin/role/list', 'Admin\\RoleListController@route');
Route::post('/admin/role/show', 'Admin\\RoleShowController@route');
Route::post('/admin/role/edit', 'Admin\\RoleEditController@route');
Route::post('/admin/role/add', 'Admin\\RoleAddController@route');
Route::post('/admin/role/del', 'Admin\\RoleDelController@route');

Route::post('/admin/menu/list', 'Admin\\MenuListController@route');
Route::post('/admin/menu/edit', 'Admin\\MenuEditController@route');
Route::post('/admin/menu/show', 'Admin\\MenuShowController@route');
Route::post('/admin/menu/add', 'Admin\\MenuAddController@route');
Route::post('/admin/menu/del', 'Admin\\MenuDelController@route');

Route::post('/admin/userrole/show', 'Admin\\UserRoleShowController@route');
Route::post('/admin/userrole/add', 'Admin\\UserRoleAddController@route');
Route::post('/admin/userrole/del', 'Admin\\UserRoleDelController@route');
Route::post('/admin/userrole/info', 'Admin\\UserRoleInfoController@route');
Route::post('/admin/userrole/edit', 'Admin\\UserRoleEditController@route');

Route::post('/admin/rolemenu/list', 'Admin\\RolemenuListController@route');
Route::post('/admin/rolemenu/add', 'Admin\\RolemenuAddController@route');


Route::post('/device/list', 'Device\\Web\\ListController@route');
Route::post('/device/detail', 'Device\\Web\\DetailController@route');
Route::post('/device/token', 'Device\\Web\\TokenController@route');
Route::post('/device/mac', 'Device\\MacListController@route');
Route::post('/device/sn/downloadurl', 'Device\\Web\\ExportController@route');
Route::post('/device/sn-task/list', 'Device\\Web\\SnTaskListController@route');
Route::post('/device/sn-task/new', 'Device\\Web\\SnTaskCreateController@route');
Route::post('/device/sn-task/ack', 'Device\\Web\\SnTaskAckController@route');
Route::get('/device/sn-task/notify', 'Device\\Web\\TaskQueueNotifyController@route');
Route::post('/device/sn/range', 'Device\\Web\\SnRangeController@route');
Route::post('/device/bsn/list', 'Device\\Web\\BsnListController@route');
Route::post('/device/bsn/unbind', 'Device\\Web\\BsnUnbindController@route');

// passport
Route::get('/home', 'HomeController@index')->name('home');
