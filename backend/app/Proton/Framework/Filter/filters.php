<?php

use Proton\Framework\Filter\JsonRequestFilter;

function is_not_empty($v)
{
    return !empty($v);
}

function is_int_string($v)
{
    return is_string($v) && preg_match('/^[0-9]+$/', $v);
}

function is_optional_of($v)
{
    return new \Proton\Framework\Filter\OptionalFilter($v);
}

function is_array_of($tmpl, $minCount = 0, $maxCount = -1)
{
    return new \Proton\Framework\Filter\ArrayFilter($tmpl, $minCount, $maxCount);
}

function is_map_of($tmpl)
{
    return new \Proton\Framework\Filter\KeyValueFilter($tmpl);
}

function is_any_of($tmpl)
{
    return new \Proton\Framework\Filter\AnyFilter($tmpl);
}

function is_equal_anyof(...$values)
{
    return new \Proton\Framework\Filter\MatchFilter($values);
}

function is_value_match(object $value, array $tmpl)
{
    $s = new JsonRequestFilter($tmpl);
    return $s->check("{INPUT}", $value);
}
