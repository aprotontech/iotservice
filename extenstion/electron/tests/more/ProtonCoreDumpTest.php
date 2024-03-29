<?php

require_once dirname(__DIR__) . '/proton_test.php';


class ProtonCoreDumpTest extends ProtonTestCase
{

    public function testDump1()
    {
        $values = [];
        $coroutine = new Proton\Electron\Coroutine(function () use (&$values) {
            for ($i = 0; $i < 3; ++$i) {
                $values[] = $i;
            }
        });

        $coroutine->resume();
        $this->assertEquals(3, count($values));
        for ($i = 0; $i < 3; ++$i) {
            $this->assertEquals($i, $values[$i]);
        }
    }

    public function testDump2()
    {
        $values = [];
        Proton\Electron\go(function () use (&$values) {
            $values[] = 1;
        });

        Proton\Electron\go(function (&$values) {
            $values[] = 2;
        }, $values);

        $this->assertEquals(1, count($values));
    }
}
