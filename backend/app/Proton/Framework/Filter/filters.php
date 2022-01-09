<?php

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

function is_equal_anyof(...$values)
{
    return new \Proton\Framework\Filter\MatchFilter($values);
}
