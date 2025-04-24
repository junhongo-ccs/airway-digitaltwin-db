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
        Schema::create('average_population', function (Blueprint $table) {
            $table->unsignedBigInteger('average_population_id')->comment('人流情報ID')->autoIncrement();
            $table->string('spatial_id',50)->comment('空間ID（Lv17）');
            $table->dateTime('from_datetime')->comment('開始日時');
            $table->dateTime('to_datetime')->nullable()->comment('終了日時');
            $table->integer('hour')->comment('時間帯(0～23)');
            $table->integer('holiday_flg')->comment('平休日別フラグ');
            $table->bigInteger('mesh_area')->comment('メッシュエリア');
            $table->integer('city_code')->comment('市区町村コード');
            $table->double('stay_average_population',12,2)->comment('平均滞在人口（当該年月の平休日単位）');
            $table->double('move_average_population',12,2)->comment('平均移動人口（当該年月の平休日単位）');
            $table->double('stay_average_population_spatial')->comment('空間内平均滞在人口');
            $table->double('move_average_population_spatial')->comment('空間内平均移動人口');
            $table->dateTime('created_at')->comment('登録日');
            $table->dateTime('updated_at')->comment('更新日');
            $table->dateTime('deleted_at')->nullable()->comment('削除日');

            $table->unique(['spatial_id', 'from_datetime', 'hour', 'holiday_flg', 'mesh_area'],'average_population_unique');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('average_population');
    }
};
