<?php

namespace Proton\Framework\Filter;

interface IFilter {
    
    public function check($prefix, $value);

    public function isOptional();
};
