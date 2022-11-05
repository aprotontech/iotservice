<?php

namespace App\Console\Commands;

use Illuminate\Console\Command;
use Illuminate\Contracts\Http\Kernel;
use Illuminate\Support\Facades\Log;
use Proton;
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
        if ($this->option('daemon')) {
            $daemon = new Daemon(-1, -1, $this);
            $daemon->start();
        }

        proton_set_logger_level(3);
        \Proton\Eletron\Runtime::setErrorHandler(function ($coroutine, $error) {
            Log::error("coroutinue($coroutine) throw error($error)");
        });

        \Proton\Eletron\go(function ($cmd) {
            $address = $cmd->option('host');
            $port = $cmd->option('port');
            Log::notice("Proton Server Listen at $address:$port");
            $server = new \Proton\Eletron\HttpServer($address, $port, function ($server, $request) {
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


        \Proton\Eletron\Runtime::start();
        return 0;
    }
}
