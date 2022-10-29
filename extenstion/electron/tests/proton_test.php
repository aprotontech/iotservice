<?php

require_once __DIR__ . '/vendor/autoload.php';

use Proton\Logger;
use Monolog\Handler\StreamHandler;

use PHPUnit\Framework\TestCase;

class ProtonTestCase extends TestCase
{
    private $logger;

    public function log()
    {
        if (!$this->logger) {
            $logger = Proton\Logger::getDefaultLogger();
            $logger->setLevel(0);
            $this->logger = $log;
        }

        return $this->logger;
    }
}

function utlog($format, ...$args)
{
    $msg = vsprintf($format, $args);
    $logger = Proton\Logger::getDefaultLogger();
    $logger->write(4, $msg);
}

if (defined('PROTON_PRINT_CORE_LOGGER') && PROTON_PRINT_CORE_LOGGER) {
    Proton\Logger::setCoreLogger(Proton\Logger::getDefaultLogger());
}
