<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

class CreateDevicesTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('devices', function (Blueprint $table) {
            $table->id();
            $table->timestamps();

            $table->char('app_id', 16)->default('')->comment('app id');
            $table->char('device_id', 40)->default('')->comment('device name');
            $table->char('secret', 32)->default('')->comment('device secret');
            $table->char('client_id', 32)->default('')->unique()->comment('unique device uuid');
            $table->char('session_id', 32)->default('')->unique()->comment('device session');
            $table->dateTime('session_timeout')->comment('session timeout');
            $table->unique(['app_id', 'device_id']);
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('devices');
    }
}
