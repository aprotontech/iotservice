<?php

require_once dirname(__DIR__) . '/proton_test.php';

class TopicTest extends ProtonTestCase
{
    public function testTopicMatch1()
    {
        $this->assertEquals(1, Proton\Electron\MqttClient::isTopicMatch("/abc", "/abc"));
        $this->assertEquals(1, Proton\Electron\MqttClient::isTopicMatch("/abc/def", "/abc/def"));
        $this->assertEquals(0, Proton\Electron\MqttClient::isTopicMatch("/abc/de", "/abc/def"));
        $this->assertEquals(0, Proton\Electron\MqttClient::isTopicMatch("/abc", "/abc/def"));
        $this->assertEquals(0, Proton\Electron\MqttClient::isTopicMatch("/abc/def", "/abc"));
    }

    public function testTopicMatch2()
    {
        $this->assertEquals(1, Proton\Electron\MqttClient::isTopicMatch("/+", "/abc"));
        $this->assertEquals(0, Proton\Electron\MqttClient::isTopicMatch("/+", "/abc/xyz"));
        $this->assertEquals(1, Proton\Electron\MqttClient::isTopicMatch("/+/xyz", "/abc/xyz"));
        $this->assertEquals(1, Proton\Electron\MqttClient::isTopicMatch("/+/xyz/+", "/abc/xyz/x"));
    }

    public function testTopicMatch3()
    {
        $this->assertEquals(1, Proton\Electron\MqttClient::isTopicMatch("/#", "/abc"));
        $this->assertEquals(1, Proton\Electron\MqttClient::isTopicMatch("/#", "/abc/xyz"));
        $this->assertEquals(1, Proton\Electron\MqttClient::isTopicMatch("sensor/#", "sensor"));
        $this->assertEquals(1, Proton\Electron\MqttClient::isTopicMatch("sensor/#", "sensor/xyz"));
        $this->assertEquals(1, Proton\Electron\MqttClient::isTopicMatch("/+/xyz/#", "/abc/xyz/mmm/nnn"));
        $this->assertEquals(0, Proton\Electron\MqttClient::isTopicMatch("/#", "abc/xyz"));
    }
}
