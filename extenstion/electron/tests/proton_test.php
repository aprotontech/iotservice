<?php

require_once __DIR__ . '/vendor/autoload.php';

use Monolog\Logger;
use Monolog\Handler\StreamHandler;

use PHPUnit\Framework\TestCase;

class ProtonTestCase extends TestCase
{
    private $logger;

    public function log()
    {
        if (!$this->logger) {
            $log = new Logger('Tester');
            $log->pushHandler((new StreamHandler('php://stdout', Logger::DEBUG, true)));
            $this->logger = $log;
        }

        return $this->logger;
    }
}
