<?php

require_once dirname(__DIR__) . '/proton_test.php';


class ProtonCoroutineLoopTest extends ProtonTestCase
{
    public function testLoopStartStop()
    {
        $values = [];
        Proton\go(function () use (&$values) {
            $values[] = "test-1";
            Proton\Runtime::stop();
            $values[] = "test-2";
        });

        Proton\Runtime::start();

        $this->assertEquals(2, count($values));
        $this->assertEquals("test-1", $values[0]);
        $this->assertEquals("test-2", $values[1]);
    }

    public function testLoopStartStopCompare()
    {
        $values = [];
        Proton\go(function () use (&$values) {
            $values[] = "test-1";
            Proton\context()->pause();
            $values[] = "test-2";
            Proton\Runtime::stop();
        });

        $this->assertEquals(1, count($values));
        $this->assertEquals("test-1", $values[0]);

        Proton\Runtime::start();

        $this->assertEquals(2, count($values));
        $this->assertEquals("test-1", $values[0]);
        $this->assertEquals("test-2", $values[1]);
    }
}
