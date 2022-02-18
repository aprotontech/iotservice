<?php

namespace Proton\Framework\Filter;

trait FilterTrait {

    protected function compositeCheck($prefix, $tmpl, $value) {
        foreach ($tmpl as $k => $v) {
            $m = $this->getValue($prefix, $k, $value);
            if (is_string($v)) { // do real check
                $r = $this->leafCheck("$prefix.$k", $v, $m);
                if ($r !== true) return $r;
            }
            else if (is_array($v)) {
                if ($m[0] === true) {
                    $r = $this->compositeCheck("$prefix.$k", $v, $m[1]);
                    if ($r !== true) return $r;
                }
                else {
                    return $m[1];
                }
            }
            else if ($v instanceof IFilter) {
                if ($v->isOptional() && $m[0] === false) continue;
                else if ($m[0] !== true) return $m[1];

                $r = $v->check("$prefix.$k", $m[1]);
                if ($r !== true) return $r;
            }
        }

        return true;
    }

    protected function leafCheck($prefix, $tmpl, $m) {
        $checkError = true;
        $canOptional = false;
        foreach (explode('|', $tmpl) as $orTmpl) {
            $err = '';
            $count = 0;
            $hasOptional = false;
            foreach (explode('&', $orTmpl) as $v) {
                $v = trim($v);
                if (!$v) continue;

                ++ $count;
                if ($v == 'optional') $hasOptional = true;
                else if ($m[0] !== true) {
                    $err = $m[1];
                    break;
                }
                else {
                    if (!call_user_func($v, $m[1])) {
                        $err = "$prefix not match $v, input is(".json_encode($m[1]).")";
                        break;
                    }
                }
            }

            if ($hasOptional) {
                if ($count != 1) {
                    throw new \Exception("$prefix's checker error: optional can't be with other condition");
                }

                $canOptional = true; // input params may be Optional
            }
            else {
                if (!$err) return true; // not found error

                $checkError = $err; // save error
            }
        }

        if ($canOptional) {
            if ($m[0] === false) {
                return true;
            }
        }

        return $checkError;
    }

    private function getValue($prefix, $k, $value) {
        if (is_array($value)) {
            if (!isset($value[$k])) {
                return [false, "Not Found $prefix.$k"];
            }
            return [true, $value[$k]];
        }
        else if (is_object($value)) {
            if (!isset($value->$k)) {
                return [false, "Not Found $prefix.$k"];
            }
            return [true, $value->$k];
        }

        return [-1, "$prefix is not an object or array"];
    }
}
