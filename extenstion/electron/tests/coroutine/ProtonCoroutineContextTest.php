<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonCoroutineContextTest extends ProtonTestCase
{
    public static $values = [];
    public function testContextStatus()
    {
        ProtonCoroutineContextTest::$values = [];
        Proton\Electron\go(function () {
            ProtonCoroutineContextTest::$values[] = "status=" . Proton\Electron\context()->status();
            for ($i = 0; $i < 3; ++$i) {
                ProtonCoroutineContextTest::$values[] = "test1-$i";
            }
        });

        $this->assertEquals(4, count(ProtonCoroutineContextTest::$values));

        $this->assertEquals("status=2", ProtonCoroutineContextTest::$values[0]);

        for ($i = 0; $i < 3; ++$i) {
            $this->assertEquals("test1-$i", ProtonCoroutineContextTest::$values[$i + 1]);
        }
    }

    public function testContextStatus2()
    {
        ProtonCoroutineContextTest::$values = [];
        $coroutine = new Proton\Electron\Coroutine(function () {
            ProtonCoroutineContextTest::$values[] = "status=" . Proton\Electron\context()->status();
        });

        $this->assertEquals(1, $coroutine->status());

        $coroutine->resume();

        $this->assertEquals(4, $coroutine->status());
    }

    public function testContextPause()
    {
        ProtonCoroutineContextTest::$values = [];
        $coroutine = new Proton\Electron\Coroutine(function () {

            ProtonCoroutineContextTest::$values[] = "test-1";
            Proton\Electron\context()->pause();
            ProtonCoroutineContextTest::$values[] = "test-2";
        });

        $this->assertEquals(0, count(ProtonCoroutineContextTest::$values));
        $coroutine->resume();
        $this->assertEquals(1, count(ProtonCoroutineContextTest::$values));
        $this->assertEquals("test-1", ProtonCoroutineContextTest::$values[0]);

        $coroutine->resume();
        $this->assertEquals(2, count(ProtonCoroutineContextTest::$values));
        $this->assertEquals("test-1", ProtonCoroutineContextTest::$values[0]);
        $this->assertEquals("test-2", ProtonCoroutineContextTest::$values[1]);
    }
}
