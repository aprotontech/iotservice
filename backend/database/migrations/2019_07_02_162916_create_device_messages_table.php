<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

class CreateDeviceMessagesTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('device_messages', function (Blueprint $table) {
            $table->bigIncrements('id');
            $table->char('app_id', 16)->default('')->comment('appId');
            $table->char('client_id', 14)->default('')->comment('SN');
            $table->char('msg_id', 20)->default('')->comment('messageId');
            $table->integer('type')->default(0)->comment('type: 0-str, 1-file');
            $table->char('protocol', 10)->default('')->comment('protocol: http/mqtt/grpc');
            $table->string('action', 64)->default('')->comment('action');
            $table->string('topic', 64)->default('')->comment('topic');
            $table->text('content')->comment('message content');
            $table->text('files')->comment('file urls');
            $table->bigInteger('recv_time')->comment('recved message time: ms');
            $table->timestamps();

            $table->index(['app_id', 'client_id', 'action', 'topic', 'created_at']);
            $table->index(['created_at', 'client_id']);
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('device_messages');
    }
}
