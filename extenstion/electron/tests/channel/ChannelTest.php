<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ChannelTest extends ProtonTestCase
{
    public function testQueueZeno()
    {
        $this->expectException(\PHPUnit\Framework\Error\Warning::class);

        $channel = new \Proton\Channel(0);
        $this->log()->info("$channel");
    }

    public function testQueueOne()
    {
        $channel = new \Proton\Channel(1);

        $values = [];
        \Proton\go(function ($channel, $test) use (&$values) {
            for ($i = 0; $i < 10; ++$i) {
                $test->assertEquals(0, $channel->push($i));
                array_push($values, "push:$i");
            }
        }, $channel, $this);


        \Proton\go(function ($channel) use (&$values) {
            for ($i = 0; $i < 10; ++$i) {
                $x = $channel->pop();
                array_push($values, "pop:$x");
            }
            \Proton\Runtime::stop();
        }, $channel);

        \Proton\Runtime::start();
        $this->assertEquals(20, count($values));
        for ($i = 0; $i < 10; ++$i) {
            $this->assertEquals("push:$i", $values[2 * $i]);
            $this->assertEquals("pop:$i", $values[2 * $i + 1]);
        }
    }

    public function testQueueFive()
    {
        $channel = new \Proton\Channel(5);

        $values = [];
        \Proton\go(function ($channel, $test) use (&$values) {
            for ($i = 0; $i < 10; ++$i) {
                $test->assertEquals(0, $channel->push($i));
                array_push($values, "push:$i");
            }
        }, $channel, $this);


        \Proton\go(function ($channel, $test) use (&$values) {
            for ($i = 0; $i < 10; ++$i) {
                $x = $channel->pop();
                array_push($values, "pop:$x");
                $test->assertEquals($i, $x);
            }
            \Proton\Runtime::stop();
        }, $channel, $this);

        \Proton\Runtime::start();
        $this->assertEquals(20, count($values));
    }
}
