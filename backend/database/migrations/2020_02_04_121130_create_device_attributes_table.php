<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateDeviceAttributesTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('device_attributes', function (Blueprint $table) {

            $table->bigIncrements('id');
            $table->string('app_id', 32)->default('')->comment('app_id');
            $table->string('client_id', 40)->default('')->comment('clientId: sn');
            $table->string('attr', 50)->default('')->comment('property key');
            $table->string('value', 100)->default('')->comment('data value');
            $table->enum('type', ['bool', 'int', 'float', 'string'])->comment('property type');
            $table->bigInteger('report_time')->default(0)->comment('device report timestamp: ms');
            $table->timestamps();

            $table->unique(['client_id', 'attr']);
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('device_attributes');
    }
}
