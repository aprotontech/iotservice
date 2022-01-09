<?php

namespace Proton\Framework\Filter;

abstract class BaseFilter implements IFilter {
    use FilterTrait;

    protected $template;
    public function __construct($tmpl) {
        $this->template = $tmpl;
    }

    public function check($prefix, $value) {
        if (is_array($this->template)) {
            if (!is_array($value) && !is_object($value)) {
                return "$prefix input value must be array or object";
            }
            return $this->compositeCheck($prefix, $this->template, $value);
        } else if ($this->template instanceof IFilter) {
            return $this->template->check($prefix, $value);
        }
        return $this->leafCheck($prefix, $this->template, [true, $value]);
    }

    public function isOptional() {
        return false;
    }

}

