<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

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
            $table->engine = 'InnoDB';

            $table->increments('id');
            $table->char('app_id', 64)->default('')->comment('app_id');
            $table->char('uuid', 64)->default('')->comment('uuid');
            $table->char('client_id', 64)->default('')->comment('client id');
            $table->char('mac', 12)->nullable()->comment('mac_address');
            $table->integer('status')->default(0)->comment('0-reserve, 1-normal');
            $table->char('session', 32)->default('')->comment('device session');
            $table->timestamp('session_timeout')->nullable()->comment('session timeout');
            $table->integer('is_online')->default(0)->comment('device online?');
            $table->timestamp('online_time')->nullable()->comment('device last online time');
            $table->timestamp('offline_time')->nullable()->comment('device last offline time');
            $table->timestamp('active_time')->nullable()->comment('device active time');

            $table->timestamps();
            $table->softDeletes();

            $table->unique(['app_id', 'uuid']);
            $table->index(['app_id', 'status']);
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
        Schema::dropIfExists('devices');
    }
}
