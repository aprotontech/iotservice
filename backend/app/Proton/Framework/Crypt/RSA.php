<?php

namespace Proton\Framework\Crypt;

class RSA
{
    private $publicKey;
    private $privateKey;

    public function __construct($publicKey, $privateKey, $salt = false)
    {
        $this->publicKey = $publicKey;
        $this->privateKey = $privateKey;
    }

    public function auth($encrypt, $rand, $secret = false)
    {
        if (strlen($encrypt) > 64) {
            return $this->decryptByPrivateKey($encrypt);
        }

        return $this->authSecret($encrypt, $rand, $secret);
    }

    public function decryptByPrivateKey($encrypt, $print = true)
    {
        $decrypted = false;
        try {
            $piKey = openssl_pkey_get_private($this->privateKey);
            $ret = openssl_private_decrypt(base64_decode($encrypt), $decrypted, $piKey);
            if (!$ret || ($print && !preg_match('/^[0-9a-zA-Z]{16,48}$/', $decrypted))) {
                $decrypted = false;
            }

            rclog_info("decrypted value=($decrypted)");
        } catch (\Exception $e) {
            rclog_exception($e);
            $decrypted = false;
        }

        return $decrypted;
    }

    public function authSecret($encrypt, $rand, $secret = false)
    {
        if (!$secret) {
            $secret = md5($this->publicKey);
        }

        $min = floor(time() / 60);
        $diff = [0, -1, 1];
        for ($i = 0; $i <= 2; ++$i) {
            $s = $diff[$i] + $min;
            $m = md5(md5($secret . $rand . $s) . 'APROTON');
            if ($m == $encrypt) return md5($encrypt);
        }

        return false;
    }

    public function decryptByPublicKey($value)
    {
    }
}
