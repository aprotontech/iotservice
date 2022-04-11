<?php

namespace Proton\Framework\Logger;

class Logger extends \Illuminate\Log\Logger
{
    private $loggerIndex = 0;
    private $loggerUUID;
    private $useLaravelLogger = true;

    protected function writeLog($level, $message, $context)
    {
        list($uuid, $index) = $this->getLoggerContext();
        $message = sprintf('[%s][%d] ', $uuid, $index) . $this->formatMessage($message);
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

    private function getLoggerContext()
    {
        if (function_exists('\Proton\context') && ($current = \Proton\context()) != null) { // enabled 
            if (!isset($current->_loggerUUID)) {
                $current->_loggerUUID = $this->newUUID();
            }
            if (!isset($current->_loggerIndex)) {
                $current->_loggerIndex = 0;
            }

            return [$current->_loggerUUID, $current->_loggerIndex++];
        }
        return [$this->getLoggerUUID(), $this->loggerIndex++];
    }

    private function getLoggerUUID()
    {
        if (!$this->loggerUUID) {
            $this->loggerUUID = $this->newUUID();
        }

        return $this->loggerUUID;
    }

    private function newUUID()
    {
        return substr(md5(getmypid() . microtime(true) . 'ProtonLogger'), 8, 16);
    }
};
