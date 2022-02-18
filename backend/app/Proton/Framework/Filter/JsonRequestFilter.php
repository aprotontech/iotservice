<?php

namespace Proton\Framework\Filter;

class JsonRequestFilter extends BaseFilter {

    public function check($prefix, $value) {
        if (!is_array($value) && !is_object($value)) {
            return "$prefix input value must be array or object";
        }
        return $this->compositeCheck($prefix, $this->template, $value);
    }

};

