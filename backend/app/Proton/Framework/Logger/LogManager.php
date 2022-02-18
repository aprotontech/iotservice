<?php

namespace Proton\Framework\Logger;

use Throwable;
use Monolog\Formatter\LineFormatter;

class LogManager extends \Illuminate\Log\LogManager
{
    const SIMPLE_FORMAT = "[%datetime%] %channel%.%level_name%: %message% %context% %extra%\n";
    const SIMPLE_DATE = "Y-m-d H:i:s.u";

    protected function formatter()
    {
        return tap(new LineFormatter(self::SIMPLE_FORMAT, self::SIMPLE_DATE, true, true), function ($formatter) {
            $formatter->includeStacktraces();
        });
    }

    protected function get($name, ?array $config = null)
    {
        try {
            return $this->channels[$name] ?? with($this->resolve($name, $config), function ($logger) use ($name) {
                return $this->channels[$name] = $this->tap($name, new Logger($logger, $this->app['events']));
            });
        } catch (Throwable $e) {
            return tap($this->createEmergencyLogger(), function ($logger) use ($e) {
                $logger->emergency('Unable to create configured logger. Using emergency logger.', [
                    'exception' => $e,
                ]);
            });
        }
    }
}
