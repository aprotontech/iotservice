<?php namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class RoleMenu extends Model
{
    protected $table = 'role_menus';

    public function role()
    {
        return $this->hasOne('App\Models\Role','id','role_id');
    }

    public function menu()
    {
        return $this->hasOne('App\Models\Menu','id', 'menu_id');
    }
}
