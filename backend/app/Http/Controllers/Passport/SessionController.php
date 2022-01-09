<?php

namespace App\Http\Controllers\Passport;

use Illuminate\Support\Facades\Auth;

use App\Http\Controllers\WebController;


class SessionController extends WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getInfo()
    {
        try {
            $user = $this->webApiAuth();
            return $this->success([
                'name' => $user->name,
                'email' => $user->email,
                'token' => ''
            ]);
        } catch (\Exception $e) {
            rclog_exception($e);
            return $this->error($e->getCode(), $e->getMessage());
        }
    }

    public function logout()
    {
        try {
            $user = $this->webApiAuth();
            rclog_info("current login user is {$user}");
        } catch (\Exception $e) {
            return $this->error($e->getCode(), $e->getMessage());
        }
        Auth::logout();
        return $this->success();
    }
}
