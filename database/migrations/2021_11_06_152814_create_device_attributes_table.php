<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

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
            $table->id();
            $table->timestamps();

            $table->char('client_id', 32)->default('')->comment('client id');
            $table->char('attr', 32)->default('')->comment('device attr name');
            $table->tinyInteger('type')->default(0)->comment('attr value type: 0-bool, 1-int, 2-double, 3-string');
            $table->string('value')->default('')->comment('attr value');
            $table->bigInteger('device_report_time')->default(0)->comment('device report time(ms)');

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
