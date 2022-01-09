<?php namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class Role extends Model
{
    protected $table = 'roles';

    public function scopeSearchAll($query, $role, $system)
    {   
        $query->where('created_by',0);

        if(!empty($role)) {
            $query->where('name', 'like',$role.'%');
        }

        if(!empty($system)) {
            $query->where('system', $system);
        }

        return $query;
    }

    public function rolemenus()
    {
        return $this->hasMany('App\Models\RoleMenu','role_id','id');
    }

    public function rolepermissions()
    {
        return $this->hasMany('App\Models\RolePermission','role_id','id');
    }
}
