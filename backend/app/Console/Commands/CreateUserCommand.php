<?php

namespace App\Console\Commands;

use Illuminate\Console\Command;

use App\Models\User;

class CreateUserCommand extends Command
{
    /**
     * The name and signature of the console command.
     *
     * @var string
     */
    protected $signature = 'user:create {email}';

    /**
     * The console command description.
     *
     * @var string
     */
    protected $description = 'Create a new user';

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
        if ($user) {
            $this->info("input user exists");
            return;
        }

        $name = substr($email, 0, strpos($email, '@'));
        $password = str_random(10);
        $this->info("NAME: $name");
        $this->info("EMAIL: $email");
        $this->info("PASSWORD: $password");
        $user = User::create([
            'name' => $name,
            'email' => $email,
            'password' => bcrypt($password),
        ]);
    }
}
