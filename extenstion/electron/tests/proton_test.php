<?php

require_once __DIR__ . '/vendor/autoload.php';

use Proton\Electron\Logger;

use PHPUnit\Framework\TestCase;

class ProtonTestCase extends TestCase
{
}

function utlog($format, ...$args)
{
    $msg = vsprintf($format, $args);
    $logger = Logger::getDefaultLogger();
    $logger->write(Logger::LEVEL_NOTICE, $msg);
}

if (defined('PROTON_PRINT_CORE_LOGGER') && PROTON_PRINT_CORE_LOGGER) {
    Logger::getDefaultLogger()->setLevel(Logger::LEVEL_DEBUG);
    Logger::setCoreLogger(Logger::getDefaultLogger());
}
