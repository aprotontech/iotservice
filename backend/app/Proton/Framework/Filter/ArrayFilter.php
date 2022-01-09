<?php

namespace Proton\Framework\Filter;

class ArrayFilter extends BaseFilter {

    private $minCount;
    private $maxCount;

    public function __construct($tmpl, $minCount = 0, $maxCount = -1) {
        parent::__construct($tmpl);
        $this->minCount = $minCount;
        $this->maxCount = $maxCount;
    }

    public function check($prefix, $value) {
        if (!is_array($value)) {
            return "$prefix is not an array";
        }

        if (count($value) < $this->minCount) {
            return "$prefix must has more than $this->minCount items";
        }

        if ($this->maxCount != -1 && count($value) > $this->maxCount) {
            return "$prefix must has less than $this->minCount items";
        }

        $i = 0;
        foreach ($value as $m) {
            $r = parent::check($prefix."[$i]", $m);
            if ($r !== true) return $r;
            ++ $i;
        }

        return true;
    }

}

