<?php

require_once dirname(__DIR__) . '/proton_test.php';


class ProtonCoroutineLoopTest extends ProtonTestCase
{
    public function testLoopStartStop()
    {
        $values = [];
        proton\go(function () use (&$values) {
            $values[] = "test-1";
            proton\runtime::stop();
            $values[] = "test-2";
        });

        proton\runtime::start();

        $this->assertEquals(2, count($values));
        $this->assertEquals("test-1", $values[0]);
        $this->assertEquals("test-2", $values[1]);
    }

    public function testLoopStartStopCompare()
    {
        $values = [];
        proton\go(function () use (&$values) {
            $values[] = "test-1";
            proton\context()->pause();
            $values[] = "test-2";
            proton\runtime::stop();
        });

        $this->assertEquals(1, count($values));
        $this->assertEquals("test-1", $values[0]);

        proton\runtime::start();

        $this->assertEquals(2, count($values));
        $this->assertEquals("test-1", $values[0]);
        $this->assertEquals("test-2", $values[1]);
    }
}
