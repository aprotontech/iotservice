<?php namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class Permission extends Model
{
    protected $table = 'permissions';

    public function scopeSearchAll($query, $permission, $system)
    {   
        if(!empty($permission)) {
            $query->where(function($query) use ($permission){
                $query->orWhere('permission', 'like', $permission.'%')
                        ->orWhere('name', 'like', $permission.'%');
            });
        }

        if(!empty($system)) {
            $query->where('system', $system);
        }

        return $query;
    }
}
