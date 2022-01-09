<?php namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class ApiConfig extends Model
{
    protected $table = 'permission_api_config';

    public function permission()
    {
        return $this->hasOne('App\Models\Permission','permission','permission')->where('system', $this->system);
    }

    public function scopeSearchAll($query, $permission, $system, $domain, $uri, $method, $serviceUrl)
    {   
        if(!empty($permission)) {
            $query->where('permission', $permission);
        }
        if(!empty($system)) {
            $query->where('system', $system);
        }
        if(!empty($domain)) {
            $query->where('request_domain', $domain);
        }
        if(!empty($uri)) {
            $query->where('request_uri', $uri);
        }
        if(!empty($method)) {
            $query->where('request_method', $method);
        }
        if(!empty($serviceUrl)) {
            $query->where('service_url', $serviceUrl);
        }

        return $query;
    }
}
