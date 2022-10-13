<?php

namespace App\Console\Commands;

use Illuminate\Console\Command;

use App\Jobs\SnTask;

class ProcessSnTaskCommand extends Command
{
    /**
     * The name and signature of the console command.
     *
     * @var string
     */
    protected $signature = 'sntask:process';

    /**
     * The console command description.
     *
     * @var string
     */
    protected $description = 'Process SnTask';

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
     * @return mixed
     */
    public function handle()
    {
        $sntask = new SnTask();
        $sntask->handle();
    }
}
