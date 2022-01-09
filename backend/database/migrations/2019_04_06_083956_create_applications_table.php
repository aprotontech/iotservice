<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateApplicationsTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('applications', function (Blueprint $table) {
            $table->engine = 'InnoDB';

            $table->increments('id');
            $table->char('appid', 32)->unique()->default('')->comment("application id");
            $table->char('name', 128)->default('')->comment('application name');
            $table->integer("creator")->default(0)->comment('the user id of who create this application. ref to: users.id');
            $table->text('description')->comment('application description');
            $table->timestamps();
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('applications');
    }
}
