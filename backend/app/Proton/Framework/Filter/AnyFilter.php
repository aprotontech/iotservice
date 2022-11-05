<?php

namespace Proton\Framework\Filter;

class AnyFilter extends BaseFilter
{

    public function __construct(array $values)
    {
        parent::__construct($values);
    }

    public function check($prefix, $value)
    {
        $match = false;
        foreach ($this->template as $v) {
            if ($v instanceof IFilter) {
                if ($v->check($prefix, $value) === true) {
                    $match = true;
                    break;
                }
            } else if (is_array($v)) {
                $filter = new JsonRequestFilter($v);
                if ($filter->check($prefix, $value) === true) {
                    $match = true;
                    break;
                }
            } else if (is_callable($v)) {
                if (call_user_func($v, $value)) {
                    $match = true;
                    break;
                }
            }
        }

        if (!$match) {
            $array_str = json_encode($this->template);
            return "$prefix is not match any filter in $array_str";
        }

        return true;
    }
}
