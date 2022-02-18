<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateDeviceActiveRecordsTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('device_active_records', function (Blueprint $table) {
            $table->engine = 'InnoDB';

            $table->bigIncrements('id');
            $table->char('app_id', 64)->default('')->comment('app_id');
            $table->char('client_id', 64)->default('')->comment('client id');
            $table->date('active_day')->comment('device active day');
            $table->char('ip', 20)->default('')->comment('device active ip');
            $table->string('country', 50)->default('')->comment('device active location: country');
            $table->string('city', 50)->default('')->comment('device active location: city');
            $table->string('address', 200)->default('')->comment('device active location: full address');
            $table->string('wifi', 100)->default('')->comment('device active record wifi');
            $table->timestamps();

            $table->unique('client_id');
            $table->index('app_id');
            $table->index('active_day');
            $table->index('ip');
            $table->index('country');
            $table->index('city');
            $table->index('wifi');
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('device_active_records');
    }
}
