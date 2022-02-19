<?php

namespace App\Console\Commands;

use Illuminate\Console\Command;
use Illuminate\Support\Str;

use App\Models\User;

class ResetUserPasswordCommand extends Command
{
    /**
     * The name and signature of the console command.
     *
     * @var string
     */
    protected $signature = 'user:reset-password {email} {--password= : new password}';

    /**
     * The console command description.
     *
     * @var string
     */
    protected $description = 'Reset User Password';

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
        $email = $this->argument('email');
        if (!preg_match('/^([0-9a-zA-Z._]|-)+@([0-9a-zA-Z._]|-)+$/', $email)) {
            $this->info("invalidate email url");
            return;
        }
        $user = User::where('email', $email)->first();
        if (!$user) {
            $this->info("input user not exists");
            return;
        }


        if ($this->option('password')) {
            $password = $this->option('password');
        } else {
            $password = Str::random(10);
        }

        $this->info("EMAIL: $email");
        $this->info("NEW-PASSWORD: $password");
        $user->password = bcrypt($password);
        $user->save();
    }
}
