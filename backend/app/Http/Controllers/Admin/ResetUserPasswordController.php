<?php

namespace App\Http\Controllers\Admin;

use App\Models\User;
use Illuminate\Http\Request;
use Illuminate\Pagination\Paginator;

class ResetUserPasswordController extends \App\Http\Controllers\WebController
{
    public $permission = 'user_create';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            "email" => "is_string & is_not_empty",
            "password" => "is_string & is_not_empty",
        ];
    }

    public function handle(Request $req)
    {
        $info = $this->reqToObject($req);

        if (!filter_var($info->email, FILTER_VALIDATE_EMAIL)) {
            return $this->error("500", "invalidate email");
        }

        if (strlen($info->password) < 8 || !preg_match('/[0-9]/', $info->password) || !preg_match('/[A-Za-z]/', $info->password)) {
            return $this->error("500", "passport length must > 8, both has digit and latter");
        }

        $user = User::where('email', $info->email)
            ->first();
        if ($user) {
            $this->error("500", "email($info->email not found user)");
        }

        $user->password = bcrypt($info->password);
        $user->save();

        return $this->success();
    }
}
