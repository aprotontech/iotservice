<?php

namespace Proton\Framework\Filter;

class MatchFilter extends BaseFilter
{

    public function __construct(array $values)
    {
        parent::__construct($values);
    }

    public function check($prefix, $value)
    {
        $match = false;
        foreach ($this->template as $v) {
            if ($value === $v) {
                $match = true;
                break;
            }
        }

        if (!$match) {
            $array_str = json_encode($this->template);
            return "$prefix is not match any elements in $array_str";
        }

        return true;
    }
}
