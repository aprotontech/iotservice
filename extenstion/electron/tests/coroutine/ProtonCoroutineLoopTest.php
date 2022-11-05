<?php

require_once dirname(__DIR__) . '/proton_test.php';


class ProtonCoroutineLoopTest extends ProtonTestCase
{
    public function testLoopStartStop()
    {
        $values = [];
        Proton\Electron\go(function () use (&$values) {
            $values[] = "test-1";
            Proton\Electron\Runtime::stop();
            $values[] = "test-2";
        });

        Proton\Electron\Runtime::start();

        $this->assertEquals(2, count($values));
        $this->assertEquals("test-1", $values[0]);
        $this->assertEquals("test-2", $values[1]);
    }

    public function testLoopStartStopCompare()
    {
        $values = [];
        Proton\Electron\go(function () use (&$values) {
            $values[] = "test-1";
            Proton\Electron\context()->pause();
            $values[] = "test-2";
            Proton\Electron\Runtime::stop();
        });

        $this->assertEquals(1, count($values));
        $this->assertEquals("test-1", $values[0]);

        Proton\Electron\Runtime::start();

        $this->assertEquals(2, count($values));
        $this->assertEquals("test-1", $values[0]);
        $this->assertEquals("test-2", $values[1]);
    }
}
