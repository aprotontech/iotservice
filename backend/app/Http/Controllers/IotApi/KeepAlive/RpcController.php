<?php

namespace App\Http\Controllers\IotApi\KeepAlive;

use Illuminate\Http\Request;

class RpcController extends \App\Http\Controllers\ApiController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'appId' => 'is_string',
            'clientId' => 'is_string',
            'command' => 'is_string',
            'content' => is_any_of([
                'is_string',
                'is_array',
                'is_object'
            ])
        ];
    }

    public function handle(Request $request)
    {
        $input = $this->reqToObject($request);

        if (!$this->isCommandValidate($input->command)) {
            return rc_error(1011, "command is invalidate");
        }

        $remote = app('keepalive')->getRemoteClient($input->appId, $input->clientId);
        $res = $remote->sendRpcCommand($input->command, $input->content);

        if ($res === false) {
            return rc_error(1010, "send command failed");
        }

        return $this->success([
            'result' => $res
        ]);
    }

    private function isCommandValidate($command)
    {
        return preg_match("/^\w+$/", $command);
    }
}
