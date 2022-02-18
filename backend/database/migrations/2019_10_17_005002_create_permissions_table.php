<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreatePermissionsTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('permissions', function (Blueprint $table) {
            $table->increments('id');
            $table->string('permission', 100)->default('')->comment('permission tag');
            $table->string('name', 100)->default('')->comment('permission show name');
            $table->string('description', 300)->default('')->comment('permission description');
            $table->string('system', 30)->comment('system name');
            $table->timestamps();

            $table->unique(['system', 'permission']);
            $table->index('permission');
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('permissions');
    }
}
