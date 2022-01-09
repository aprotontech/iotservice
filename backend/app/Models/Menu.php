<?php namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class Menu extends Model
{
    protected $table = 'menus';

    public function scopeSearchAll($query, $title, $system, $type='', $url='')
    {   
        if (!empty($title)) {
            $query->where('title', 'like', '%'.$title.'%');
        }

        if (!empty($system)) {
            $query->where('system', $system);
        }

        if (!empty($type)) {
            $query->where('type', $type);
        }

        if (empty($url)) {
            $query->where('url', 'like', '%'.$url.'%');
        }
        
        return $query;
    }
}
