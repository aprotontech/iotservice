<?php namespace Proton\Request;

class LaravelRequest implements IRequest
{
    private $rawRequest;
    public function __construct(Request $req)
    {
        $this->rawRequest = $req;
    }

    public function reqToObject($handle)
    {
        $request = $this->rawRequest;
        $m = false;
        switch (strtoupper($request->method())) {
        case 'POST':
            if (($key = $handle->getApiFormKey())) {
                $s = $request->input($key);
            }
            else {
                $s = $request->getContent();
            }
            @ $m = json_decode($s);
            break;
        case 'GET':

            break;
        default:
            break;
        }
        
        return $m;
    }
}
