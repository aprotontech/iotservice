<?php

require_once dirname(__DIR__) . '/proton_test.php';

function child1()
{
    for ($i = 0; $i < 3; ++$i) {
        ProtonCoroutinePauseTest::$values[] = "child1-$i";
        if ($i == 1) {
            Proton\Electron\pause();
        }
    }
}

function child2()
{
    for ($i = 0; $i < 3; ++$i) {
        ProtonCoroutinePauseTest::$values[] = "child2-$i";
        if ($i == 1) {
            Proton\Electron\pause();
        }
    }
}


class ProtonCoroutinePauseTest extends ProtonTestCase
{
    public static $values = [];
    public function testSimplePause()
    {
        $values = [];
        Proton\Electron\go(function () use (&$values) {

            for ($i = 0; $i < 3; ++$i) {
                $values[] = $i;
                if ($i == 1) {
                    Proton\Electron\pause();
                }
            }
        });

        $this->assertEquals(2, count($values));
        for ($i = 0; $i < 2; ++$i) {
            $this->assertEquals($i, $values[$i]);
        }
    }


    public function testComplexPause()
    {
        ProtonCoroutinePauseTest::$values = [];
        Proton\Electron\go(function () {
            ProtonCoroutinePauseTest::$values[] = "parent-1";
            Proton\Electron\go('child1');
            ProtonCoroutinePauseTest::$values[] = "parent-2";
            Proton\Electron\go('child2');
        });

        $values = ProtonCoroutinePauseTest::$values;

        $this->assertEquals(6, count($values));
        $this->assertEquals("parent-1", $values[0]);
        $this->assertEquals("child1-0", $values[1]);
        $this->assertEquals("child1-1", $values[2]);
        $this->assertEquals("parent-2", $values[3]);
        $this->assertEquals("child2-0", $values[4]);
        $this->assertEquals("child2-1", $values[5]);
    }

    public function testSimpleResume()
    {
        $tvalues = [];
        $coroutine = new Proton\Electron\Coroutine(function () use (&$tvalues) {
            for ($i = 0; $i < 3; ++$i) {
                $tvalues[] = "test-$i";
            }
        });

        $coroutine->resume();

        $this->assertEquals(3, count($tvalues));
        for ($i = 0; $i < 3; ++$i) {
            $this->assertEquals("test-$i", $tvalues[$i]);
        }
    }

    public function testResume2()
    {
        ProtonCoroutinePauseTest::$values = [];
        $coroutine = new Proton\Electron\Coroutine('child1');

        $coroutine->resume();

        $this->assertEquals(2, count(self::$values));
        for ($i = 0; $i < 2; ++$i) {
            $this->assertEquals("child1-$i", self::$values[$i]);
        }

        $coroutine->resume();
        $this->assertEquals(3, count(self::$values));
        for ($i = 0; $i < 3; ++$i) {
            $this->assertEquals("child1-$i", self::$values[$i]);
        }
    }
}
