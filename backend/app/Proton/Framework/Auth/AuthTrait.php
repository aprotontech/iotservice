<?php 

namespace Proton\Framework\Auth;

use Illuminate\Http\Request;

trait AuthTrait
{
    protected function doAuth($request, $rcAuth)
    {
        if (is_array($rcAuth)) {
            $r = rc_success();
            foreach ($rcAuth as $auth) {
                $r = $this->doAuth($request, $auth);
                if ($r->isError()) return $r;
            }
        }
        else if (method_exists($this, $rcAuth)) {
            $r = call_user_func(array($this, $rcAuth), $request);
        }
        else if (function_exists($rcAuth)) {
            $r = call_user_func($rcAuth, $request);
        }
        else if (class_exists($rcAuth)) {
            $m = new $rcAuth;
            $r = $m->auth($request);
        }
        else $r = rc_error(500, "invalidate auth type($rcAuth)");

        return $r;
    }

}

