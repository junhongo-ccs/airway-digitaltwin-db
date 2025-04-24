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
        Schema::create('weather_forecast', function (Blueprint $table) {
            $table->unsignedBigInteger('weather_forecast_id')->comment('気象予報情報ID')->autoIncrement();
            $table->string('met_elements')->comment('気象要素');
            $table->dateTime('from_datetime')->comment('開始日時');
            $table->dateTime('to_datetime')->nullable()->comment('終了日時');
            $table->string('basetime', 255)->comment('気象予測情報発表時間');
            $table->string('minute')->comment('指定時間');
            $table->text('contents')->comment('予測データ');
            $table->string('lat_interval')->comment('緯度の間隔');
            $table->string('lon_interval')->comment('経度の間隔');
            $table->string('lat_start')->comment('北西端の緯度');
            $table->string('lon_start')->comment('北西端の経度');
            $table->string('lat_end')->comment('南東端の緯度');
            $table->string('lon_end')->comment('南東端の経度');
            $table->string('mesh')->comment('メッシュサイズ');
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
        Schema::dropIfExists('weather__forecast');
    }
};
