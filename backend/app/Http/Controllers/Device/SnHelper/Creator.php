<?php

namespace App\Http\Controllers\Device\SnHelper;

use Proton\Framework\Crypt\Utils as CryptUtils;

class Creator
{
    private $snLength;
    private $snUseHex;

    public function __construct($length, $hex = false)
    {
        $this->snLength = $length;
        $this->snUseHex = $hex;
    }

    public function createClients($prefix, $start, $count, $is_assign_mac = false)
    {
        $len = $this->snLength - strlen($prefix);
        $fmt = "$prefix%0{$len}" . ($this->snUseHex ? 'x' : 'd');
        rclog_debug("client format($fmt)");

        $maxId = $this->snUseHex ? pow(16, $len) : pow(10, $len);
        if ($start + $count >= $maxId) {
            rclog_warning("max than maxId($maxId), start($start), count($count)");
            return [];
        }

        $clients = [];
        for ($i = 0; $i < $count; ++$i) {
            $sn = sprintf($fmt, $i + $start);

            list($pubKey, $priKey) = CryptUtils::allocRsaKeyPair();
            array_push($clients, (object)[
                'uuid' => $sn,
                'mac' => '',
                'clientId' => $sn,
                'publicKey' => $pubKey,
                'privateKey' => $priKey,
                'secret' => md5($pubKey . $priKey . 'aproton.tech' . microtime(true) . getmypid())
            ]);
        }

        return $clients;
    }
}
