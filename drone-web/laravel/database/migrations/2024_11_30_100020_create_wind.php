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
        Schema::create('wind', function (Blueprint $table) {
            $table->unsignedBigInteger('wind_id')->comment('風速・天候情報ID')->autoIncrement();
            $table->string('type', 255)->comment('風速天候種');
            $table->dateTime('from_datetime')->comment('開始日時');
            $table->dateTime('to_datetime')->nullable()->comment('終了日時');
            $table->double('lon_start')->comment('格子点の経度');
            $table->double('lat_start')->comment('格子点の緯度');
            $table->double('lon_interval')->comment('格子点の間隔(経度)');
            $table->double('lat_interval')->comment('格子点の間隔(緯度)');
            $table->integer('lon_count')->comment('経度の格子点数');
            $table->integer('lat_count')->comment('緯度の格子点数');
            $table->text('values')->comment('気象情報');
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
        Schema::dropIfExists('wind');
    }
};
