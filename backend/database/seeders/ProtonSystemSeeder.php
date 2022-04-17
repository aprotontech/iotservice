<?php

namespace Database\Seeders;

use Exception;
use Illuminate\Database\Seeder;
use Illuminate\Support\Facades\DB;

class ProtonSystemSeeder extends Seeder
{
    /**
     * Run the database seeds.
     *
     * @return void
     */
    public function run()
    {
        $this->newSystem();
        $this->newAdminUser();
        $this->newAdminRole();
        $this->newMenus();
        $this->newTestApp();
        $this->newSnTask();
    }

    private function newSystem()
    {
        try {
            $system = new \App\Models\System();
            $system->name = 'aproton';
            $system->desc = 'aproton';
            $system->save();
        } catch (Exception $e) {
            if (!$this->isDuplicate($e)) {
                echo $e->getMessage() . "\n";
                rclog_exception($e);
            }
        }
    }

    private function newAdminUser()
    {
        try {
            $user = new \App\Models\User();
            $user->name = 'admin';
            $user->email = 'admin@aproton.tech';
            $user->password = bcrypt("admin123");
            $user->save();
        } catch (Exception $e) {
            if (!$this->isDuplicate($e)) {
                echo $e->getMessage() . "\n";
                rclog_exception($e);
            }
        }
    }

    private function newAdminRole()
    {
        try {
            $role = new \App\Models\Role();
            $role->name = 'admin';
            $role->system = 'aproton';
            $role->desc = 'admin role';
            $role->save();
        } catch (Exception $e) {
            if (!$this->isDuplicate($e)) {
                echo $e->getMessage() . "\n";
                rclog_exception($e);
            }
        }

        $adminRole = \App\Models\Role::where('name', 'admin')->where('system', 'aproton')->first();

        $permissions = [
            ['admin_manager', 'admin', 'admin permission'],
            ['device_view', 'device_view', 'view device information'],
            ['user_create', 'user_create', 'create user'],
            ['app_manager', 'app_manager', 'manager applications'],
            ['bsn_view', 'bsn_view', 'view bsn information'],
            ['bsn_bind', 'bsn_bind', 'bind bsn and device'],
            ['device_export', 'device_export', 'export devices'],
            ['sn_task', 'sn_task', 'create sn task'],
        ];

        foreach ($permissions as $m) {
            try {
                $permission = new \App\Models\Permission();
                $permission->permission = $m[0];
                $permission->name = $m[1];
                $permission->system = 'aproton';
                $permission->description = $m[2];
                $permission->save();
            } catch (Exception $e) {
                if (!$this->isDuplicate($e)) {
                    echo $e->getMessage() . "\n";
                    rclog_exception($e);
                }
            }

            try {
                $pr = new \App\Models\RolePermission();
                $pr->permission = $m[0];
                $pr->role_id = $adminRole->id;
                $pr->system = 'aproton';
                $pr->data_rule = '*';
                $pr->save();
            } catch (Exception $e) {
                if (!$this->isDuplicate($e)) {
                    echo $e->getMessage() . "\n";
                    rclog_exception($e);
                }
            }
        }
    }

    private function newMenus()
    {
        $menus = [
            [1,  "Device",       "#/iot/devicelist",        0,  "0",    0,   '{"mainMenuId":"device"}'],
            [2,  "Admin",        "#/permission",             0,  "0",    2,  '{"mainMenuId":"permission"}'],
            [3,  "UserAccount",  "",                         2,  "0",    0,  '["el-icon-setting"]'],
            [4,  "Users",        "accountlist",              3,  "0",    0,  '{}'],
            [5,  "Permission",   "",                         2,  "0",    1,  '["el-icon-setting"]'],
            [6,  "Systems",      "systems",                  5,  "0",    0,  '{}'],
            [7,  "Roles",        "roles",                    5,  "0",    1,  '{}'],
            [8,  "Permissions",  "permissions",              5,  "0",    2,  '{}'],
            [9,  "Menus",        "menus",                    5,  "0",    3,  '{}'],
            [10, "DeviceManager", "",                        1,  "0",    0,  '{}'],
            [11, "Devices",       "devicelist",             10,  "0",    0,  '{}'],
            [12, "Export",        "deviceexport",           10,  "0",    1,  '{}'],
            [13, "Application",   "#/app/",                 0,  "0",    1,   '{"mainMenuId":"app"}'],
            [14, "Manager",       "",                       13,  "0",    0,  '{}'],
            [15, "List",          "applist",                14,  "0",    0,  '{}'],
            [16, "Apply",         "sntasklist",             10,  "0",    2,  '{}'],
        ];
        $rootMenuIds = [];
        $menuObjects = [];
        foreach ($menus as $m) {
            $menu = \App\Models\Menu::where('title', $m[1])
                ->where('system', 'aproton')
                ->first();
            if (!$menu) {
                try {
                    $menu = new \App\Models\Menu();
                    $menu->title = $m[1];
                    $menu->url = $m[2];
                    $menu->pid = $m[3] == 0 ? 0 : $menuObjects[$m[3]]->id;
                    $menu->type = $m[4];
                    $menu->sort = $m[5];
                    $menu->system = 'aproton';
                    $menu->attrs = $m[6];
                    $menu->save();
                } catch (Exception $e) {
                    if (!$this->isDuplicate($e)) {
                        echo $e->getMessage() . "\n";
                        rclog_exception($e);
                    }
                }
            }

            $menuObjects[$m[0]] = $menu;

            array_push($rootMenuIds, $menu->id);
        }

        $adminUser = \App\Models\User::where('name', 'admin')->first();
        $adminRole = \App\Models\Role::where('name', 'admin')->where('system', 'aproton')->first();

        try {
            $ur = new \App\Models\UserRole();
            $ur->user_id = $adminUser->id;
            $ur->role_id = $adminRole->id;
            $ur->system = 'aproton';
            $ur->data_rule = '*';
            $ur->save();
        } catch (Exception $e) {
            if (!$this->isDuplicate($e)) {
                echo $e->getMessage() . "\n";
                rclog_exception($e);
            }
        }

        foreach ($rootMenuIds as $menuId) {
            try {
                $ur = new \App\Models\RoleMenu();
                $ur->menu_id = $menuId;
                $ur->role_id = $adminRole->id;
                $ur->system = 'aproton';
                $ur->save();
            } catch (Exception $e) {
                if (!$this->isDuplicate($e)) {
                    echo $e->getMessage() . "\n";
                    rclog_exception($e);
                }
            }
        }
    }

    private function newTestApp()
    {
        $adminUser = \App\Models\User::where('name', 'admin')->first();
        try {
            $app = new \App\Models\Application();
            $app->appid = 'test';
            $app->name = 'TestApp';
            $app->creator = $adminUser->id;
            $app->description = "Test Application";
            $app->save();
        } catch (Exception $e) {
            if (!$this->isDuplicate($e)) {
                echo $e->getMessage() . "\n";
                rclog_exception($e);
            }
        }

        try {
            $m = json_decode(file_get_contents(__DIR__ . '/test-app-keys.json'), false);
            DB::table('app_keys')
                ->insert([
                    'app_id' => 'test',
                    'salt' => '',
                    'public_key' => $m->pubKey,
                    'private_key' => $m->priKey,
                    'secret' => '7045298f456cea6d7a4737c62dd3b89e'
                ]);
        } catch (Exception $e) {
            if (!$this->isDuplicate($e)) {
                echo $e->getMessage() . "\n";
                rclog_exception($e);
            }
        }
    }

    private function newSnTask()
    {
        $adminUser = \App\Models\User::where('name', 'admin')->first();
        try {
            if (DB::table('sn_tasks')->count() > 0) {
                return;
            }

            $taskTemplte = [
                'app_id' => 'test',
                'name' => 'Example-Test-SN-Task',
                'prefix' => 'F1010F1F',
                'start' => 'F1010F1F000000',
                'end' => 'F1010F1F000009',
                'count' => 10,
                'status' => 1, // accepted
                'type' => 1, // auto-regist
                'creator' => $adminUser->name,
                'description' => 'Example Sn Task',
                'acker' => $adminUser->name,
                'acked_at' => date('Y-m-d H:i:s'),
                'created_at' => date('Y-m-d H:i:s'),
                'updated_at' => date('Y-m-d H:i:s'),
                'assign_mac' => 0
            ];

            $autoRegistTask = $taskTemplte;
            DB::table('sn_tasks')
                ->insert($autoRegistTask);

            $job = new \App\Jobs\SnTask();
            $job->handle(); // $autoRegistTask
        } catch (Exception $e) {
            if (!$this->isDuplicate($e)) {
                echo $e->getMessage() . "\n";
                rclog_exception($e);
            }
        }
    }

    private function isDuplicate($e)
    {
        return strpos($e->getMessage(), 'Duplicate entry') !== false;
    }
}
