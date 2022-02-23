<?php

namespace Proton\Framework\Crypt;

class Utils
{
    public static function allocRsaKeyPair($bits = 1024)
    {
        $config = ["private_key_bits" => $bits];
        $res = openssl_pkey_new($config);
        openssl_pkey_export($res, $privkey);
        $d = openssl_pkey_get_details($res);
        $pubkey = $d['key'];
        return [$pubkey, $privkey];
    }
}
