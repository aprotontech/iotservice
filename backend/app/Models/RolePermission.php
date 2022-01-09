<?php namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class RolePermission extends Model
{
    protected $table = 'role_permissions';

    public function permission()
    {
        return $this->hasOne('App\Models\Permission','permission','permission')->where('system', $this->system);
    }

    public function setDataRuleAttribute($val)
    {
        if(empty($val) || strtolower($val) == 'all') {
            $val = '*';
        }
        $this->attributes['data_rule'] = $val;
    }

}
