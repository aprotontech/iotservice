<?php

namespace App\Http\Controllers\Device\Web\SnHelper;

use DB;

class Creator
{
    private $snLength;
    private $snUseHex;
    private $keyFolder;

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
        if ($is_assign_mac) {
            $mac_list = $this->getMacList($count);
        } else {
            $mac_list = null;
        }

        for ($i = 0; $i < $count; ++$i) {
            $sn = sprintf($fmt, $i + $start);
            $mac = ($mac_list == null ? '' : $mac_list[$i]);

            list($pubKey, $priKey) = $this->createKeys($sn);
            array_push($clients, (object)[
                'uuid' => $sn,
                'mac' => $mac,
                'clientId' => $sn,
                'publicKey' => $pubKey,
                'privateKey' => $priKey,
                'secret' => md5($pubKey . $priKey . 'rc' . microtime(true) . getmypid())
            ]);
        }

        return $clients;
    }

    private function getMacList($count)
    {
        rclog_debug("[getMacList] count=" . $count);
        $mac_list = array();
        $total = 0;
        while ($total < $count) {
            $record = DB::table('mac_allocate')->where('alloc_address', '<>', 'end_address')->lockForUpdate()->first();
            if (!$record) {
                throw new \Exception("mac address is not enough.");
            }

            $id = $record->id;
            $start_address = $record->start_address;
            $alloc_address = $record->alloc_address;
            $end_address = $record->end_address;
            $s = hexdec($start_address);
            $alloc = hexdec($alloc_address);
            $e = hexdec($end_address);
            //            rclog_debug("[test] id=" . $id . " s=" . $s . " e=" . $e . " alloc=" . $alloc);

            //从未分配过
            if ($alloc == 0) {
                $alloc = $s - 1;
            }
            $tmp = min($count, $e - $alloc);
            for ($i = 0; $i < $tmp; $i++) {
                $alloc++;
                $address = strtoupper(dechex($alloc));
                array_push($mac_list, $address);
                //                rclog_debug("[test] alloc=" . $alloc . " address=" . $address . " count=" . count($mac_list));
            }
            $total += $tmp;
            //            rclog_debug("[test] tmp=" . $tmp . " total=" . $total);

            $address = strtoupper(dechex($alloc));
            DB::table('mac_allocate')->where('id', '=', $id)
                ->update(['alloc_address' => $address]);
        }

        return $mac_list;
    }

    private function getKeyFolder()
    {
        if (!$this->keyFolder) {
            $this->keyFolder = storage_path() . '/app/keys';
            if (!file_exists($this->keyFolder)) {
                @mkdir($this->keyFolder);
            }
        }

        return $this->keyFolder;
    }

    private function createKeys($sn)
    {
        $config = ["private_key_bits" => 1024];
        $res = openssl_pkey_new($config);
        openssl_pkey_export($res, $privkey);
        $d = openssl_pkey_get_details($res);
        $pubkey = $d['key'];
        return [$pubkey, $privkey];
    }
}
