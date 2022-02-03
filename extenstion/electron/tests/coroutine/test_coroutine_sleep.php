<?php

require_once dirname(__DIR__) . '/proton_test.php';


class ProtonCoroutineSleepTest extends ProtonTestCase
{
    public function testSleep()
    {
        $use_tm = 0;
        proton\go(function () use (&$use_tm) {
            $start = microtime(true);
            proton\sleep(1000);
            $end = microtime(true);
            $use_tm = floor(($end - $start) * 1000);

            proton\runtime::stop();
        });

        proton\runtime::start();

        $this->assertTrue(abs(1000 - $use_tm) < 100);
    }
}
