<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateDeviceDailyAliveRecordsTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('device_daily_alive_records', function (Blueprint $table) {
            $table->bigIncrements('id');

            $table->string('app_id', 64)->default('')->comment('app_id');
            $table->string('client_id', 64)->default('')->comment('client_id');
            $table->date('day')->comment('alive date');
            $table->integer('ative')->default(0)->comment('is active today?');
            $table->timestamps();

            $table->unique(['day', 'client_id']);
            $table->index(['day', 'app_id']);
            $table->index('client_id');
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('device_daily_alive_records');
    }
}
