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
        Schema::create('weather_now_spatial', function (Blueprint $table) {
            $table->unsignedBigInteger('id')->comment('サロゲートキー')->autoIncrement();
            $table->unsignedBigInteger('weather_now_id')->comment('気象実況情報ID');
            $table->string('spatial_id',50)->comment('空間ID（Lv17）');
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
        Schema::dropIfExists('weather_now_spatial');
    }
};
