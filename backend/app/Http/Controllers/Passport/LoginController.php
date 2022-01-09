<?php

namespace App\Http\Controllers\Passport;

use Auth;

use App\Models\User;
use App\Http\Controllers\Controller;
use Illuminate\Http\Request;

class LoginController extends Controller
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'account' => 'is_string',
            'password' => 'is_string'
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);
        $email = $info->account;
        if (!strpos($info->account, '@')) { // not email
            $user = User::where('name', $info->account)->first();
            if (!$user) {
                return $this->error("404", "invalidate account($info->account)");
            }
            $email = $user->email;
        }

        if (!Auth::attempt(['email' => $email, 'password' => $info->password])) {
            return $this->error("402", "login failed, check account or password");
        }

        return $this->success();
    }
}
