<?php

require_once dirname(__DIR__) . '/proton_test.php';


class ProtonCoroutineSleepTest extends ProtonTestCase
{
    public function testSleep()
    {
        $use_tm = 0;
        Proton\Electron\go(function () use (&$use_tm) {
            $start = microtime(true);
            Proton\Electron\sleep(1000);
            $end = microtime(true);
            $use_tm = floor(($end - $start) * 1000);

            Proton\Electron\Runtime::stop();
        });

        Proton\Electron\Runtime::start();

        $this->assertTrue(abs(1000 - $use_tm) < 100);
    }

    public function testManyCoroutines()
    {
        $values = [];
        $total_count = 100;
        for ($i = 0; $i < $total_count; ++$i) {
            Proton\Electron\go(function () use (&$values, $i) {
                $values[$i] = 1;
                Proton\Electron\sleep(100);
                $values[$i] = 2;
            });
        }

        Proton\Electron\go(function () {
            Proton\Electron\sleep(1000);
            Proton\Electron\Runtime::stop();
        });

        Proton\Electron\Runtime::start();

        $this->assertEquals($total_count, count($values));
        for ($i = 0; $i < $total_count; ++$i) {
            $this->assertEquals(2, $values[$i]);
        }
    }

    public function testManySleeps()
    {
        $count = 0;
        Proton\Electron\go(function () use (&$count) {
            for ($i = 0; $i < 100; ++$i) {
                Proton\Electron\sleep(10);

                ++$count;
            }
            Proton\Electron\Runtime::stop();
        });

        Proton\Electron\Runtime::start();

        $this->assertEquals(100, $count);
    }
}
