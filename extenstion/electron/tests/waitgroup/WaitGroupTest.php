<?php

require_once dirname(__DIR__) . '/proton_test.php';

class WaitGroupTest extends ProtonTestCase
{

    public function testWaitFirst()
    {
        $func = function ($ms) {
            return Proton\Electron\go(function ($ms) {
                Proton\Electron\sleep($ms);
                utlog("sleep $ms ms done");
                return $ms;
            }, $ms);
        };
        $wg = new Proton\Electron\WaitGroup();
        $wg->append($func(200), $func(100));
        Proton\Electron\go(function ($wg) {
            $co = $wg->wait();
            utlog("wait done");
            $this->assertNotNull($co);
            $this->assertEquals(100, $co->getReturnValue());

            Proton\Electron\sleep(200);
            Proton\Electron\Runtime::stop();
        }, $wg);

        Proton\Electron\Runtime::start();

        $wg->close();

        $this->assertNull(Proton\Electron\Runtime::getLastError());
    }
}
