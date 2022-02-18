<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateMenusTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('menus', function (Blueprint $table) {
            $table->increments('id');

            $table->string('title', 100)->comment('menu title');
            $table->string('url', 100)->comment('menu name');
            $table->integer('pid')->default(0)->comment('menu pid');
            $table->string('type', 50)->default('')->comment('type, 0-?');
            $table->integer('sort')->default(0)->comment('sort, less');
            $table->string('system', 50)->default('');
            $table->string('desc', 100)->default('');
            $table->text('attrs')->comment('json object');

            $table->timestamps();
            $table->index(['system', 'type']);
            $table->unique(['title', 'pid']);
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('menus');
    }
}
