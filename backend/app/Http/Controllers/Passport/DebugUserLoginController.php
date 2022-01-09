<?php

namespace App\Http\Controllers\Passport;

use Auth;

use App\Models\User;
use App\Http\Controllers\WebController;
use Illuminate\Http\Request;

class DebugUserLoginController extends WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public function login(Request $request)
    {
        $user = $this->getDebugUser($request);
        if (!$user) {
            return $this->error("500", "not found debuger user");
        }

        Auth::login($user);

        return $this->success([
            'name' => $user->name,
            'email' => $user->email,
            'token' => ''
        ]);
    }

    private function getDebugUser($request)
    {
        $name = $request->input("name");
        if ($name) {
            return User::where('name', $name)->first();
        }
        return null;
    }
}
