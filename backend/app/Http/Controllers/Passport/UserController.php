<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;

class UserController extends Controller
{
    /**
     * Create a new controller instance.
     *
     * @return void
     */
    public function __construct()
    {
        $this->middleware('auth');
    }

    public function editInfo(Request $request)
    {
        $user = $this->webApiAuth();

        @ $m = json_decode($request->getContent());
        if (!empty($m->name)) $user->name = $m->name;

        $user->save();

        return $this->apiSuccess();
    }

    public function resetPassWord()
    {
    }
}
