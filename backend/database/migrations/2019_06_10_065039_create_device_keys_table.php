<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateDeviceKeysTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('device_keys', function (Blueprint $table) {
            $table->engine = 'InnoDB';

            $table->increments('id');
            $table->char('client_id', 64)->default('')->comment('client id');

            $table->char('salt', 64)->default('')->comment('salt to encrypt public/private key');
            $table->char('secret', 32)->default('')->comment('secret key of the device');
            $table->string('public_key', 2048)->default('')->comment('public key of the device');
            $table->string('private_key', 2048)->default('')->comment('public key of the device');

            $table->timestamps();

            $table->unique(['client_id']);
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('device_keys');
    }
}
