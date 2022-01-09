<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateSnTasksTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('sn_tasks', function (Blueprint $table) {
            $table->engine = 'InnoDB';

            $table->increments('id');
            $table->string('name')->default('')->comment('task name');
            $table->string('app_id', 64)->default('')->comment('task app_id');
            $table->string('prefix', 32)->default('')->comment('sn prefix');
            $table->string('start', 32)->default('')->comment('first sn');
            $table->string('end', 32)->default('')->comment('last sn');
            $table->integer('count')->default(0)->comment('count');
            $table->tinyInteger('assign_mac')->default(0)->comment('is assgin mac');
            $table->integer('status')->default(0)->comment('status, 0-req,1-allow,2-gen,3-done,4-deny');
            $table->integer('type')->default(0)->comment('0-auto,1-release');
            $table->text('description')->comment('description');
            $table->string('creator')->default('')->comment('creator');
            $table->string('acker')->default('')->comment('reviewer');
            $table->timestamp('acked_at')->nullable()->comment('ack time');

            $table->timestamps();
            $table->index(['app_id']);
            $table->index(['prefix']);
            $table->index(['start', 'end']);
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('tasks');
    }
}
