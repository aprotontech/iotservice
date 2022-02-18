<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateGrantDataTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('grant_data', function (Blueprint $table) {
            $table->increments('id');

            $table->integer('grant_type')->default(0)->comment('1 - to role, 2 - to permission');
            $table->string('grant_id',50)->default('')->comment('=user_roles.id');
            $table->string('data',100)->comment('appId or any other data');

            $table->timestamps();
            $table->index(['grant_type', 'grant_id', 'data']);
            $table->index('grant_id');
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('grant_data');
    }
}
