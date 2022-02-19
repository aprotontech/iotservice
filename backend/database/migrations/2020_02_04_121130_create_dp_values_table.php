<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateDpValuesTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('dp_values', function (Blueprint $table) {
            $table->engine = 'InnoDB';

            $table->bigIncrements('id');
            $table->string('client_id', 40)->default('')->comment('clientId: sn');
            $table->string('property', 50)->default('')->comment('property key');
            $table->string('device_val', 100)->default('')->comment('data value');
            $table->bigInteger('report_time')->default(0)->comment('device report timestamp: ms');
            $table->timestamps();

            $table->unique(['client_id', 'property']);
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('dp_values');
    }
}
