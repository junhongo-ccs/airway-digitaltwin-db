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
        Schema::create('area_object_masters', function (Blueprint $table) {
            $table->unsignedBigInteger('area_object_id')->comment('第三者立ち入り管理情報ID')->autoIncrement();
            $table->string('area_id')->comment('第三者立ち入り管理エリアID');
            $table->dateTime('from_datetime')->comment('開始日時');
            $table->dateTime('to_datetime')->nullable()->comment('終了日時');
            $table->integer('instrusion_status')->comment('侵入状態');
            $table->dateTime('timestamp')->comment('タイムスタンプ');
            $table->text('coordinates')->comment('第三者立入管理範囲の配列');
            $table->text('traffics')->comment('侵入検知対象情報');
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
        Schema::dropIfExists('area_object_masters');
    }
};
