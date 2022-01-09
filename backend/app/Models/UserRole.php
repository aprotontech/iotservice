<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class UserRole extends Model
{
    protected $table = 'user_roles';

    public function role()
    {
        return $this->hasOne('App\Models\Role', 'id', 'role_id');
    }

    public function user()
    {
        return $this->hasOne('App\Models\User', 'id', 'user_id');
    }

    public function getDataRuleAttribute($val)
    {
        return empty($val) || $val == '*' ? 'all' : $val;
    }

    public function setDataRuleAttribute($val)
    {
        if (empty($val) || strtolower($val) == 'all' || strtolower($val) == '*') {
            $val = 'all';
        }
        $this->attributes['data_rule'] = $val;
    }
}
