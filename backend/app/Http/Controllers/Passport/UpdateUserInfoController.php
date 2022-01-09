<?php

namespace App\Http\Controllers\WebApi;

use Illuminate\Http\Request;

use App\Models\User;


class UpdateUserInfoController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'email' => 'is_string & is_not_empty',
            'name' => 'is_string & is_not_empty',
            'newPassword' => 'optional | is_string',
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);

        if (isset($info->newPassword) && (strlen($info->newPassword) < 8 || !preg_match('/[0-9]/', $info->newPassword) || !preg_match('/[A-Za-z]/', $info->newPassword))) {
            return $this->error(10, "passport length must > 8, both has digit and latter");
        }
        $user = $this->webApiAuth();

        if (!$user) {
            return $this->error(403, "not found webapi token");
        }

        if ($user->email != $info->email) {
            return $this->error("403", "input email($info->email) not match user's email($user->email)");
        }

        $user = User::where('email', $info->email)->first();
        if ($user) {
            if (isset($info->newPassword)) {
                $user->password = bcrypt($info->newPassword);
            }
            $user->name = $info->name;
            $user->save();
        }

        return $this->success();
    }
}
