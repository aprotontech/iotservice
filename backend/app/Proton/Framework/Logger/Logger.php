<?php

namespace Proton\Framework\Logger;

class Logger extends \Illuminate\Log\Logger
{
    private $loggerIndex = 0;
    private $loggerUUID;
    private $useLaravelLogger = true;

    protected function writeLog($level, $message, $context)
    {
        $message = sprintf('[%s][%d] ', $this->getLoggerUUID(), $this->loggerIndex++) . $this->formatMessage($message);
        $this->fireLogEvent($level, $message, $context);
        if (is_null($this->useLaravelLogger)) {
            $this->useLaravelLogger = env('PROTON_LARAVEL_LOGGER', false);
        }
        if ($this->useLaravelLogger) {
            $this->logger->{$level}($message, $context);
        } else {
            \SeasLog::log(constant('SEASLOG_' . strtoupper($level)), $message, $context);
        }
    }

    private function getLoggerUUID()
    {
        if (!$this->loggerUUID) {
            $this->loggerUUID = substr(md5(getmypid() . microtime(true) . 'ProtonLogger'), 8, 16);
        }

        return $this->loggerUUID;
    }
};
