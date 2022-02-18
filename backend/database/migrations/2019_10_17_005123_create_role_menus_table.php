<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateRoleMenusTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('role_menus', function (Blueprint $table) {
            $table->increments('id');

            $table->integer('role_id')->default(0)->comment('=roles.id');
            $table->integer('menu_id')->default(0)->comment('=menus.id');
            $table->string('system',50)->default('')->comment('system name');

            $table->timestamps();

            $table->index(['role_id', 'system']);
            $table->unique(['menu_id', 'role_id']);
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('role_menus');
    }
}
