<?php

namespace Proton\Framework\Filter;

class KeyValueFilter extends BaseFilter
{

    public function __construct($tmpl)
    {
        parent::__construct($tmpl);
    }

    public function check($prefix, $value)
    {
        if (!is_object($value)) {
            return "$prefix is not an object";
        }

        foreach ($value as $key => $val) {
            $r = parent::check($prefix . "$key", $val);
            if ($r !== true) return $r;
        }

        return true;
    }
}
