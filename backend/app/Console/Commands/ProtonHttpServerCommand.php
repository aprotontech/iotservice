<?php

namespace App\Console\Commands;

use Illuminate\Console\Command;
use Illuminate\Contracts\Http\Kernel;
use Illuminate\Support\Facades\Log;
use Proton;

class ProtonHttpServerCommand extends Command
{
    /**
     * The name and signature of the console command.
     *
     * @var string
     */
    protected $signature = 'proton:httpserver {action: start/stop/restart} {--address=0.0.0.0 : listen address} {--port=8080 : listen port}';

    /**
     * The console command description.
     *
     * @var string
     */
    protected $description = 'Command description';

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
        proton_set_logger_level(3);
        \Proton\Runtime::setErrorHandler(function ($coroutine, $error) {
            Log::error("coroutinue($coroutine) throw error($error)");
        });

        \Proton\go(function ($cmd) {
            $address = $cmd->option('address');
            $port = $cmd->option('port');
            Log::notice("Proton Server Listen at $address:$port");
            $server = new \Proton\HttpServer($address, $port, function ($server, $request) {
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

        \Proton\runtime::start();
        return 0;
    }
}
