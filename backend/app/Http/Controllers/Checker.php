<?php

namespace App\Http\Controllers;

use DB;
use Cache;
use Illuminate\Http\Request;

class Checker
{
    // $permissions = ['permission1', 'permission2']
    // $data = [{"key":"appId","value":["007cd3983760"]}]
    // delete $data = [
    //     'appId' => ['appId1', 'appId2']
    // ]
    public function auth($userId, $system, $permissions, $rawData)
    { //
        $data = [];
        foreach ($rawData as $one) {
            if (empty($data[$one->key])) {
                $data[$one->key] = [];
            }
            $data[$one->key] = array_merge($data[$one->key], $one->value);
        }
        $checkResult = [];
        foreach ($permissions as $permission) {
            $checkResult[$permission] = false;
        }

        $grantPermissions = $this->getUserPermissions($userId, $system, $permissions);
        foreach ($grantPermissions as $perm => $bindRoles) {
            $dataRights = $this->checkDataRights($bindRoles, $data);
            $checkResult[$permission] = $dataRights;
        }

        return $checkResult;
    }

    public function getGrantData($userId, $system, $permissions, $dataRule)
    {
        if (!$dataRule || !$permissions) return [];

        $grantPermissions = $this->getUserPermissions($userId, $system, $permissions);
        if (count($grantPermissions) != count($permissions)) {
            rclog_debug("permissions count not match");
            return [];
        }

        $data = [];
        foreach ($dataRule as $r) {
            $data[$r] = (object)[
                'first' => true,
                'hasAll' => false,
                'values' => []
            ];
        }

        $result = [];
        foreach ($grantPermissions as $perm => $bindRoles) {
            $m = [];
            foreach ($dataRule as $r) {
                $m[$r] = [];
            }
            foreach ($bindRoles as $role) {
                $type = $role->data_rule;
                if (isset($m[$type])) {
                    $m[$type] = array_merge($m[$type], $role->data);
                } else if (strtolower($type) == 'all') {
                    foreach ($dataRule as $type) {
                        $m[$type] = array_merge($m[$type], $role->data);
                    }
                }
            }

            foreach ($dataRule as $r) {
                $this->updateGrantData($data[$r], $m[$r]);
            }
        }

        $grantData = [];
        foreach ($data as $k => $v) {
            $grantData[$k] = $v->hasAll ? ['*'] :
                array_diff($v->values, ['*']);
        }

        return $grantData;
    }

    private function updateGrantData($rule, $newData)
    {
        $thisHasAll = in_array('*', $newData);
        if ($rule->first) {
            $rule->first = false;
            $rule->hasAll = $thisHasAll;
            $rule->values = $thisHasAll ? ['*'] : array_diff($newData, ['*']);
        } else if ($rule->hasAll && $thisHasAll) {
            $rule->values = ['*'];
        } else if ($rule->hasAll) {
            $rule->hasAll = false;
            $rule->values = $newData;
        } else if ($thisHasAll) { // do no'thing
            $rule->hasAll = false;
        } else {
            $m = array_intersect($this->values, $newData);
            $this->values = array_diff($m, ['*']);
        }
    }

    private function checkDataRights($roles, $data)
    {
        if (!$data) return true;
        if (!$roles) {
            rclog_info("not found any roles");
            return false;
        }

        $match = [];
        foreach ($data as $k => $vList) {
            $match[$k] = false;
        }

        foreach ($roles as $role) { // get role data
            if ($role->data_rule == 'all' || $role->data_rule == '*') { // match all
                foreach ($data as $rule => $vList) {
                    if ($match[$rule]) continue;
                    rclog_debug(var_export($role->data, true));
                    $data[$rule] = $this->getLeftData($role->data, $vList);
                    if (empty($data[$rule])) $match[$rule] = true;
                }
            } else if (isset($match[$role->data_rule]) && !$match[$role->data_rule]) {
                $rule = $role->data_rule;
                $data[$rule] = $this->getLeftData($role->data, $data[$rule]);
                if (empty($data[$rule])) $match[$rule] = true;
            }
        }

        foreach ($match as $k => $v) {
            if (!$v) {
                rclog_info("not found any role match data($k) value=" . json_encode($data[$k]));
                return false;
            }
        }

        return true;
    }

    private function getLeftData($roleData, $vList)
    {
        if (in_array('*', $roleData) || empty($vList)) {
            return [];
        } else {
            $vList = array_diff($vList, $roleData);
        }

        return $vList;
    }

    private function getUserPermissions($userId, $system, $permissions)
    {
        $grantRoles = $this->getUserGrantRoles($userId, $system);
        $roleIds = array_keys($grantRoles);

        $grants = $this->getRolePermissions($system, $roleIds, $permissions);

        $userPermissions = [];
        foreach ($grants as $r) {
            $userPermissions[$r->permission][] = $grantRoles[$r->role_id];
        }

        return $userPermissions;
    }

    private function getRolePermissions($system, $roleIds, $permissions)
    {
        $grants = DB::table('role_permissions')
            ->whereIn('role_id', $roleIds)
            ->where('system', $system)
            ->whereIn('permission', $permissions)
            ->get();

        return $grants;
    }

    private function getUserGrantRoles($userId, $system)
    {
        $cacheKey = "/user/$userId/roles";
        $roles = Cache::get($cacheKey);
        $roles = [];
        if (!$roles) {
            $list = DB::table('user_roles')
                ->where('system',  $system)
                ->where('user_id', $userId)
                ->get();

            $roles = [];
            $grants = [];
            foreach ($list as $row) {
                $row->data = ($row->data_rule == 'all' || $row->data_rule == '*') ? ['*'] : [];
                $roles[$row->role_id] = $row;
                $grants[$row->id] = $row;
            }

            $gdata = DB::table('grant_data')
                ->where('grant_type', 1)
                ->whereIn('grant_id', array_keys($grants))
                ->get();

            foreach ($gdata as $gd) {
                $role = $grants[$gd->grant_id];
                $role->data[] = $gd->data;
            }
        }

        return $roles;
    }
}
