<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateRolePermissionsTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('role_permissions', function (Blueprint $table) {
            $table->increments('id');

            $table->integer('role_id')->default(0)->comment('=roles.id');
            $table->string('permission', 50)->default('')->comment('permission name');
            $table->string('data_rule', 30)->comment('date type: appId or any ?');
            $table->string('system', 30)->comment('system name');

            $table->timestamps();

            $table->unique(['role_id', 'system', 'permission']);
            $table->index(['system', 'permission']);
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
        Schema::dropIfExists('role_permissions');
    }
}
