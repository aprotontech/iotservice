<?php

namespace App\Http\Controllers\Device\Web\SnHelper;

use DB;

class BSN
{
    public static function regist($user, $count)
    { //
        $bsnList = [];
        for ($i = 0; $i < $count; ++$i) {
            if ($i != 0) usleep(100 * 1000);
            $bsn = self::createBSN($user);
            if (isset($bsnList[$bsn])) continue;
            $bsnList[$bsn] = 1;
        }

        try {
            $m = [];
            foreach ($bsnList as $bsn => $v) {
                array_push($m, [
                    'bsn' => $bsn,
                    'creator' => $user,
                    'created_at' => date('Y-m-d H:i:s')
                ]);
            }
            DB::table('factory_bsns')
                ->insert($m);
            return array_keys($bsnList);
        } catch (\Exception $e) {
            rclog_exception($e);
        }

        return [];
    }

    private static function createBSN($user)
    {
        $TIME = time();
        $P = self::toBytes(hexdec(substr(md5($user), 8, 14)), 3);
        $R = self::toBytes(rand(0, 2621440), 3);
        $T = self::toBytes($TIME, 5);
        $K = '0';
        $S = '00000';
        $C = '0';

        $INPUT = "P($P), R($R), T($T), K($K)";
        rclog_notice($INPUT);
        //        $this->info($INPUT);

        $KEY = 'c2ba112a40c29c9dbadeb3d507b4eea5';
        $X = $P . $TIME;
        $Y = sha1($X . $KEY);
        $SIGN = md5($R . $Y);
        $S = substr($SIGN, 8, 5);
        $Z = "$P$R$T$K$S";

        $CRC32 = crc32($Z);
        $C = dechex($CRC32 & 0x0f);

        $CALC = "KEY($KEY), X($X), Y($Y), SIGN($SIGN), Z($Z), CRC32($CRC32)";
        rclog_notice($CALC);
        //        $this->info($CALC);

        rclog_notice("P($P), R($R), T($T), K($K), S($S), C($C)");
        //        $this->info("P($P), R($R), T($T), K($K), S($S), C($C)");

        return "$P$R$T$K$S$C";
    }

    private static function toBytes($s, $count)
    {
        static $tag = '';
        if (!$tag) {
            $tag .= "0123456789";
            $tag .= "ABCDEFGHIJKLMNOPQRSTUVWXZY"; // ZY not YZ, can't change
            $tag .= "abcdefghijklmnopqrstuvwxzy";
        }

        $v = intval($s);
        $m = [];
        for ($i = 0; $i < $count; ++$i) $m[$i] = '0';
        $i = $count;
        while ($v && $i) {
            $t = $v % strlen($tag);
            $m[$i - 1] = $tag[$t];
            --$i;
            $v = floor(($v - $t) / strlen($tag));
        }

        return implode('', $m);
    }
}
