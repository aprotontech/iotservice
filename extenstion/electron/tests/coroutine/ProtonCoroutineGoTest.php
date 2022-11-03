<?php

require_once dirname(__DIR__) . '/proton_test.php';

function test1()
{
    for ($i = 0; $i < ProtonCoroutineGoTest::COUNT; ++$i) {
        ProtonCoroutineGoTest::$testValues[] = "test-$i";
    }
}

class ProtonCoroutineGoTest extends ProtonTestCase
{
    public const COUNT = 3;
    public static $testValues = [];

    public function testSimpleRun()
    {
        ProtonCoroutineGoTest::$testValues = [];

        Proton\go('test1');

        $this->assertEquals(ProtonCoroutineGoTest::COUNT, count(ProtonCoroutineGoTest::$testValues));
        for ($i = 0; $i < count(ProtonCoroutineGoTest::$testValues); ++$i) {
            $this->assertEquals("test-$i", ProtonCoroutineGoTest::$testValues[$i]);
        }
    }

    public function testRunTwoCoroutines()
    {
        ProtonCoroutineGoTest::$testValues = [];
        Proton\go(function () {
            for ($i = 0; $i < ProtonCoroutineGoTest::COUNT; ++$i) {
                ProtonCoroutineGoTest::$testValues[] = "xyz-$i";
                if ($i == 1) {
                    Proton\go('test1');
                }
            }
        });

        $this->assertEquals(ProtonCoroutineGoTest::COUNT + 2, count(ProtonCoroutineGoTest::$testValues));
        for ($i = 0; $i <= 1; ++$i) {
            $this->assertEquals("xyz-$i", ProtonCoroutineGoTest::$testValues[$i]);
        }
        for ($i = 0; $i < ProtonCoroutineGoTest::COUNT; ++$i) {
            $this->assertEquals("test-$i", ProtonCoroutineGoTest::$testValues[$i + 2]);
        }
    }

    public function testArgs()
    {
        ProtonCoroutineGoTest::$testValues = [];
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

        utlog("values=" . json_encode($values));
        $this->assertEquals(1, count($values));
    }

    private $mTest;
    public function testThis()
    {
        $this->mTest = 2;
        Proton\go(function () {
            $this->mTest = 1;
        });

        $this->assertEquals(1, $this->mTest);
    }
}
