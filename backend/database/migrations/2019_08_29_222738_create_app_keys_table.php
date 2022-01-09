<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateAppKeysTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('app_keys', function (Blueprint $table) {
            $table->engine = 'InnoDB';

            $table->increments('id');

            $table->char('app_id', 32)->unique()->default('')->comment("application id");
            $table->char('salt', 64)->default('')->comment('salt to encrypt public/private key');
            $table->char('secret', 32)->default('')->comment('secret key of the appId');
            $table->string('public_key', 2048)->default('')->comment('public key of the appId');
            $table->string('private_key', 2048)->default('')->comment('public key of the appId');

            $table->timestamps();
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('app_keys');
    }
}
