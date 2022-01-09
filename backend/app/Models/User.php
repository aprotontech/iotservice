<?php

namespace App\Models;

use Illuminate\Foundation\Auth\User as Authenticatable;


class User extends Authenticatable
{
    protected $fillable = [
        'name', 'email', 'password',
    ];

    protected $hidden = [
        'password', 'remember_token',
    ];

    public function apps()
    {
        return $this->hasMany('App\Application', 'creator', 'id');
    }

    public function roles()
    {
        return $this->belongsToMany('App\Models\Role', 'user_roles', 'user_id', 'role_id')
            ->withPivot('system', 'data_rule', 'created_at');
    }

    public function userroles()
    {
        return $this->hasMany('App\Models\UserRole', 'user_id', 'id');
    }

    public function scopeMaster($query, $account)
    {
        if (!empty($account)) {
            $query->where(function ($query) use ($account) {
                $query->orWhere('email', 'like', '%' . $account . '%')
                    ->orWhere('name', 'like', '%' . $account . '%');
            });
        }

        return $query;
    }
}
