<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateUserRolesTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('user_roles', function (Blueprint $table) {
            $table->increments('id');

            $table->integer('user_id')->default(0)->comment('=users.id');
            $table->integer('role_id')->default(0)->comment('=roles.id');
            $table->string('data_rule', 30)->comment('data_rule=date_rules.rule');
            $table->string('system', 30)->comment('system name');

            $table->timestamps();

            $table->index(['user_id', 'system', 'role_id']);
            $table->unique(['user_id', 'role_id']);
            $table->index('role_id');
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('user_roles');
    }
}
