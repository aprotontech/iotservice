<?php

require_once dirname(__DIR__) . '/proton_test.php';


class TimeTest extends ProtonTestCase
{
    public function testAfter()
    {
        Proton\Electron\go(function () {

            utlog("before after");
            $co = Proton\Electron\after(500);
            utlog("after done");
            $this->assertNotNull($co);
            Proton\Electron\sleep(1000);
            $this->assertEquals(Proton\Electron\Coroutine::STATUS_STOPED, $co->status());
            Proton\Electron\Runtime::stop();
            utlog("after stoped");
        });

        Proton\Electron\Runtime::start();
    }
}
