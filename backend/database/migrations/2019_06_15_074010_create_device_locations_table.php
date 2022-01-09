<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateDeviceLocationsTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('device_locations', function (Blueprint $table) {
            $table->engine = 'InnoDB';

            $table->increments('id');
            $table->char('client_id', 64)->default('')->comment('client id');
            $table->char('city', 32)->default('')->comment('device city name');

            $table->double('latitude', 12, 6)->default(0)->comment('device location latitude');
            $table->double('longitude', 12, 6)->default(0)->comment('device location longitude');
            $table->char('country', 32)->default('')->comment('device country name');
            $table->char('province', 32)->default('')->comment('device province name');
            $table->char('ip', 20)->default('')->comment('report ip');
            $table->string('address')->default('')->comment('device address');

            $table->timestamps();

            $table->unique(['client_id']);
            $table->index('city');
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('device_locations');
    }
}
