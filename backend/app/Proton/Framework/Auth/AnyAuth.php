<?php 

namespace Proton\Framework\Auth;

use Illuminate\Http\Request;

class AnyAuth
{
    use AuthTrait;

    private $authList;

    public function __construct($auths)
    {
        $this->authList = $auths;
    }

    public function auth(Request $request)
    {
        foreach ($this->authList as $auth) {
        }
    }
};

