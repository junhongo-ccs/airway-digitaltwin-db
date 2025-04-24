<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        Schema::create('flight_prohibited_area_object_masters', function (Blueprint $table) {
            $table->unsignedBigInteger('flight_prohibited_area_object_id')->comment('飛行禁止エリア情報ID')->autoIncrement();
            $table->string('flight_prohibited_area_id',255)->comment('飛行禁止エリアID');
            $table->dateTime('from_datetime')->comment('開始日時');
            $table->dateTime('to_datetime')->nullable()->comment('終了日時');
            $table->string('name',255)->comment('飛行禁止エリア名');
            $table->text('range')->comment('飛行禁止範囲');
            $table->string('detail',255)->comment('飛行詳細');
            $table->string('url',255)->comment('説明URL');
            $table->integer('flight_prohibited_area_type_id')->comment('飛行禁止エリア種別');
            $table->integer('status')->comment('処理ステータス');
            $table->dateTime('created_at')->comment('登録日');
            $table->dateTime('updated_at')->comment('更新日');
            $table->dateTime('deleted_at')->nullable()->comment('削除日');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('flight_prohibited_area_object_masters');
    }
};
