<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateDpDefinesTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('dp_defines', function (Blueprint $table) {
            $table->engine = 'InnoDB';

            $table->increments('id');
            $table->string('app_id', 50)->default('')->comment('appId');
            $table->string('property', 50)->default('')->comment('property id/key');
            $table->string('name', 50)->default('')->comment('property name(read name)');
            $table->boolean('read_only')->default(0)->comment('readonly?');
            $table->boolean('data_real')->default(1)->comment('0-virtual, 1-real');
            $table->string('data_type', 10)->default('str')->comment('date type: str/bool/int/enum/float/byte');
            $table->string('unit_str', 50)->default('')->comment('unit to show');
            $table->string('unit_read', 50)->default('')->comment('unit to pronunciation');
            $table->string('bool_read', 50)->default('')->comment('bool pronunciation. split by ",". first-> true, second->false');
            $table->boolean('is_sound')->default(0)->comment('pronunciation?');
            $table->decimal('max_num', 50, 10)->default(0)->comment('max number');
            $table->decimal('min_num', 50, 10)->default(0)->comment('min number');
            $table->string('emu_str', 100)->default('')->comment('enum values. split by ","');
            $table->string('default_val', 100)->default('')->comment('default value');
            $table->string('desc', 300)->default('')->comment('description');

            $table->timestamps();
            $table->unique(['app_id', 'property']);
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('dp_defines');
    }
}
