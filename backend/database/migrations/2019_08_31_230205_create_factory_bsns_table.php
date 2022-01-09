<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateFactoryBsnsTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('factory_bsns', function (Blueprint $table) {
            $table->engine = 'InnoDB';

            $table->increments('id');
            $table->char('bsn', '20')->default('')->comment('bsn');
            $table->string('sn', 20)->nullable()->comment('sn binded');
            $table->string('mac', 20)->nullable()->comment('mac binded');
            $table->string('factor', 20)->default('')->comment('factory');
            $table->string('creator')->default('')->comment('creator user email');
            $table->string('binder')->default('')->comment('bind user email');
            $table->timestamps();

            $table->unique('bsn');
            $table->unique('mac');
            $table->unique('sn');
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('factory_bsns');
    }
}
