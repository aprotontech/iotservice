<?php

namespace App\Console\Commands;

use Illuminate\Console\Command;
use Illuminate\Contracts\Http\Kernel;
use Illuminate\Support\Facades\Log;
use Proton\Electron;
use Proton\Framework\Command\Daemon;

class ProtonHttpServerCommand extends Command
{
    /**
     * The name and signature of the console command.
     *
     * @var string
     */
    protected $signature = 'proton:httpserver {action : start/stop/restart} {--host=0.0.0.0 : listen host} {--port=8080 : listen port} {--daemon : run at backgroud}';

    /**
     * The console command description.
     *
     * @var string
     */
    protected $description = 'Start Proton HttpServer';

    /**
     * Create a new command instance.
     *
     * @return void
     */
    public function __construct()
    {
        parent::__construct();
    }

    /**
     * Execute the console command.
     *
     * @return int
     */
    public function handle()
    {
        if ($this->option('daemon')) {
            $daemon = new Daemon(-1, -1, $this);
            $daemon->start();
        }

        Electron\Logger::getDefaultLogger()->setLevel(Electron\Logger::LEVEL_DEBUG);
        Electron\Logger::setCoreLogger(Electron\Logger::getDefaultLogger());
        Electron\Runtime::setErrorHandler(function ($coroutine, $error) {
            Log::error("coroutinue($coroutine) throw error($error)");
        });

        Electron\go(function ($cmd) {
            $address = $cmd->option('host');
            $port = $cmd->option('port');
            Log::notice("Proton Server Listen at $address:$port");
            $server = new Electron\HttpServer($address, $port, function ($server, $request) {
                // TODO: optimize later
                $kernel = $server->app->make(Kernel::class);

                $adaptor = new ProtonAdaptor($kernel);

                $response = $adaptor->handle($request);

                $request->end($response['StatusCode'], $response['Body'], $response['Headers']);

                $adaptor->terminate();
            });

            $server->app = app();
            $server->start();
        }, $this);


        Electron\Runtime::start();
        return 0;
    }
}
