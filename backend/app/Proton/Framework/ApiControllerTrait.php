<?php

namespace Proton\Framework;

use Illuminate\Http\Request;
use Illuminate\Http\Response;

require_once __DIR__ . '/func.php';

trait ApiControllerTrait
{

    public function route(Request $request)
    {
        $tmRoute = microtime(true);
        $rc = $this->getReqRC($request);
        $rc->time = (object)[
            'route' => $tmRoute,
            'start' => -1,
            'finish' => -1
        ];
        try {
            rclog_debug('[START] ' . $request->path());
            $r = $this->handleRequest($request);
            rclog_debug('[FINISH]');
        } catch (\Exception $e) {
            $rc->start = microtime(true);
            $rc->finish = microtime(true);
            throw $e;
        }

        $tmFinish = microtime(true);
        $rc->time->finish = $tmFinish;
        try {
            $tmStart = defined('LARAVEL_START') ? LARAVEL_START : $tmRoute;
            $rc->start = $tmStart;
            $responseStr = "";
            $code = 200;
            if ($r instanceof Error) { // date('Y-m-d H:i:s', $tmRoute), floor(1000 * ($tmRoute - floor($tmRoute)))
                $responseStr = "$r";
                $code = 200;
            } else if ($r instanceof Response) {
                $responseStr = $r->getContent();
                $code = $r->getStatusCode();
            }

            rclog_notice(
                "[CLIENT-REQUEST] start(%d), route(%d), use(%dms), total(%dms), url(%s), request(%s), response(%s), code(%d)",
                floor(1000 * $tmStart),
                floor(1000 * $tmRoute),
                floor(($tmFinish - $tmRoute) * 1000),
                floor(($tmFinish - $tmStart) * 1000),
                $request->url(),
                json_encode($this->reqToObject($request)),
                $responseStr,
                $code
            );
        } catch (\Exception $e) {
            error_log(rc_exception_message($e));
        }

        return $r;
    }

    protected function handleRequest($request)
    {
        if (method_exists($this, 'getFilter')) {
            $filter = new Filter\JsonRequestFilter($this->getFilter());
            $r = $filter->check('{REQUEST}', $this->reqToObject($request));
            if ($r !== true) return $this->error(3, $r);
        } else if (method_exists($this, 'rawFilter')) {
            $r = $this->rawFilter($request);
            if ($r->isError()) {
                return $r;
            }
        }

        if (property_exists($this, 'rcAuth')) {
            $r = $this->_auth($request, $this->rcAuth);
            if ($r->isError()) {
                return $r;
            }
        }

        return $this->handle($request);
    }

    protected function _auth($request, $rcAuth)
    {
        $auth = function ($request, $rcAuth) {
            if (is_array($rcAuth)) {
                $r = rc_success();
                foreach ($rcAuth as $auth) {
                    $r = $this->_auth($request, $auth);
                    if ($r->isError()) return $r;
                }
            } else if (method_exists($this, $rcAuth)) {
                $r = call_user_func(array($this, $rcAuth), $request);
            } else if (function_exists($rcAuth)) {
                $r = call_user_func($rcAuth, $request);
            } else if (class_exists($rcAuth)) {
                $m = new $rcAuth;
                $r = $m->auth($request);
            } else {
                $r = rc_error(500, "invalidate auth type($rcAuth)");
            }
            return $r;
        };

        $r = call_user_func($auth, $request, $rcAuth);
        return $r;
    }

    protected function reqToObject($request)
    {
        $m = false;
        switch (strtoupper($request->method())) {
            case 'POST':
                if ($this->getApiFormKey()) {
                    $s = $request->input($this->getApiFormKey());
                } else {
                    $s = $request->getContent();
                }
                @$m = json_decode($s);
                break;
            case 'GET':
                $m = $request->all();
                if (is_array($m)) $m = (object) $m;
                break;
            default:
                break;
        }

        return $m;
    }

    protected function getReqRC($request)
    {
        if (empty($request->rc)) {
            $request->rc = (object)[];
        }

        return $request->rc;
    }

    protected function getApiFormKey()
    {
        if (!empty($this->apiFormKey)) {
            return $this->apiFormKey;
        }

        return false;
    }

    public function success($data = false)
    {
        return (new Error())->success($data);
    }

    public function error($rc, $msg)
    {
        return new Error($rc, $msg);
    }
}
