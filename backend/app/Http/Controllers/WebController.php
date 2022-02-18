<?php

namespace App\Http\Controllers;

use Illuminate\Support\Facades\Auth;

use App\Exceptions\UserNotLoginException;

use Illuminate\Http\Request;

class WebController extends Controller
{
    protected $rcAuth = 'webApiPermission';

    use AppTrait;

    protected function webApiAuth($throw = true)
    {
        $user = Auth::user();
        if (!$user && $throw) {
            throw new UserNotLoginException();
        }

        return $user;
    }

    protected function webApiPermission(Request $request)
    {
        $user = Auth::user();
        if (!$user) {
            throw new UserNotLoginException();
        }

        if (property_exists($this, 'permission') && $this->permission) {
            $system = env('PERMISSION_SYSTEM_NAME', 'aproton');
            list($permissions, $data) = $this->getRequestParams($request, $this->permission);

            $checker = new Checker();
            $results = $checker->auth($user->id, $system, $permissions, $data);
            foreach ($results as $permission => $v) {
                if ($v) continue;

                $sdata = json_encode($data);
                return $this->error("403", "input user($user->email) has no permission($permission) of data($sdata)");
            }
        }

        return $this->success([
            'userId' => 'USER-' . $user->id,
            'name' => $user->name,
            'email' => $user->email,
        ]);
    }

    protected function getRequestParams(Request $request, $template)
    {
        $body = (array)$this->reqToObject($request);
        if (is_string($template)) {
            $tmpList = explode(';', $template);
            if (count($tmpList) > 2) throw new \Exception("configed permission($template) invalidate");
            $permissions = trim($tmpList[0]) ? explode(',', $tmpList[0]) : [];
            if (count($tmpList) > 1 && trim($tmpList[1])) $params = explode(',', $tmpList[1]);
            else $params = [];

            $template = [
                'permissions' => $permissions,
                'params' => $params
            ];
        }

        $m = [];
        foreach ($template['params'] as $k) {
            $c = $this->caclParams($body, $k);
            array_push($m, (object)[
                'key' => $c[0],
                'value' => $c[1]
            ]);
        }

        return [$template['permissions'], $m];
    }

    private function caclParams($body, $calc)
    {
        $t = explode('=', $calc);
        if (count($t) == 1) {
            $calc = trim($calc);
            $v = $body[$calc];
            if (!is_array($v)) $v = [$v];
            return [$calc, $v];
        } else if (count($t) > 2) {
            throw new \Exception("invalidate permission value($calc)");
        }

        $k = trim($t[0]);
        $c = trim($t[1]);

        if (preg_match('/^(\w+)\s*\((\w+)\)$/', $c, $match)) {
            rclog_debug("match");
            $func = $match[1];
            $key = $match[2];

            $v = $this->{$func}($key, $body[$key]);
        } else {
            $v = $body[$c];
        }

        if (!is_array($v)) $v = [$v];
        return [$k, $v];
    }
}
