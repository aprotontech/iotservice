<?php

require_once dirname(__DIR__) . '/proton_test.php';

function test1()
{
    for ($i = 0; $i < ProtonGoTest::COUNT; ++$i) {
        ProtonGoTest::$testValues[] = "test-$i";
    }
}

class ProtonGoTest extends ProtonTestCase
{
    public const COUNT = 3;
    public static $testValues = [];

    public function testSimpleRun()
    {
        ProtonGoTest::$testValues = [];

        Proton\go('test1');

        $this->assertEquals(ProtonGoTest::COUNT, count(ProtonGoTest::$testValues));
        for ($i = 0; $i < count(ProtonGoTest::$testValues); ++$i) {
            $this->assertEquals("test-$i", ProtonGoTest::$testValues[$i]);
        }
    }

    public function testRunTwoCoroutines()
    {
        ProtonGoTest::$testValues = [];
        Proton\go(function () {
            for ($i = 0; $i < ProtonGoTest::COUNT; ++$i) {
                ProtonGoTest::$testValues[] = "xyz-$i";
                if ($i == 1) {
                    Proton\go('test1');
                }
            }
        });

        $this->assertEquals(ProtonGoTest::COUNT + 2, count(ProtonGoTest::$testValues));
        for ($i = 0; $i <= 1; ++$i) {
            $this->assertEquals("xyz-$i", ProtonGoTest::$testValues[$i]);
        }
        for ($i = 0; $i < ProtonGoTest::COUNT; ++$i) {
            $this->assertEquals("test-$i", ProtonGoTest::$testValues[$i + 2]);
        }
    }

    public function testArgs()
    {
        ProtonGoTest::$testValues = [];
        $v1 = 3;
        $v2 = 4;
        $v3 = &$v1;


        Proton\go(function ($v1, $v3) use (&$v2) {
            for ($i = 0; $i < 3; ++$i) {
                ++$v1;
                ++$v3;
            }
            $v2 = 0;
        }, $v1, $v3);

        $this->assertEquals(3, $v1);
        $this->assertEquals(0, $v2);
        $this->assertEquals(3, $v3);
    }

    public function testArgRef()
    {
        $values = [];
        Proton\go(function () use (&$values) {
            $values[] = 1;
        });

        $this->assertEquals(1, count($values));
    }

    public function testComplexArgs()
    {
        $values = [];
        Proton\go(function () use (&$values) {
            $values[] = 1;
            Proton\go(function (&$values) {
                $values[] = 2;
            }, $values);
        });

        $this->log()->info("values", $values);
        $this->assertEquals(1, count($values));
    }
}
