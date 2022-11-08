<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ChannelTest extends ProtonTestCase
{
    public function testQueueZeno()
    {
        $this->expectException(\PHPUnit\Framework\Error\Warning::class);

        $channel = new \Proton\Electron\Channel(0);
        utlog("$channel");
    }

    public function testEasy()
    {
        $channel = new \Proton\Electron\Channel(1);

        $this->testEasyValue = null;

        \Proton\Electron\go(function ($channel) {
            $this->testEasyValue = $channel->pop();
        }, $channel);

        \Proton\Electron\go(function ($channel) {
            $channel->push("push:10");
        }, $channel);

        \Proton\Electron\go(function () {
            Proton\Electron\sleep(100);
            \Proton\Electron\Runtime::stop();
        });

        \Proton\Electron\Runtime::start();

        $this->assertEquals("push:10", $this->testEasyValue);
    }

    public function testQueueOne()
    {
        $channel = new \Proton\Electron\Channel(1);

        $values = [];
        \Proton\Electron\go(function ($channel) use (&$values) {
            for ($i = 0; $i < 10; ++$i) {
                $this->assertEquals(0, $channel->push($i));
                array_push($values, "push:$i");
            }
        }, $channel);


        \Proton\Electron\go(function ($channel) use (&$values) {
            for ($i = 0; $i < 10; ++$i) {
                $x = $channel->pop();
                array_push($values, "pop:$x");
            }
            \Proton\Electron\Runtime::stop();
        }, $channel);

        \Proton\Electron\Runtime::start();
        $this->assertEquals(20, count($values));
        for ($i = 0; $i < 10; ++$i) {
            $this->assertEquals("push:$i", $values[2 * $i]);
            $this->assertEquals("pop:$i", $values[2 * $i + 1]);
        }
    }

    public function testQueueFive()
    {
        $channel = new \Proton\Electron\Channel(5);

        $values = [];
        \Proton\Electron\go(function ($channel) use (&$values) {
            for ($i = 0; $i < 10; ++$i) {
                $this->assertEquals(0, $channel->push($i));
                array_push($values, "push:$i");
            }
        }, $channel);


        \Proton\Electron\go(function ($channel) use (&$values) {
            for ($i = 0; $i < 10; ++$i) {
                $x = $channel->pop();
                array_push($values, "pop:$x");
                $this->assertEquals($i, $x);
            }
            \Proton\Electron\Runtime::stop();
        }, $channel);

        \Proton\Electron\Runtime::start();
        $this->assertEquals(20, count($values));
    }
}
