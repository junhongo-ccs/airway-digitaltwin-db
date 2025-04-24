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
        Schema::create('flight_prohibited_area_objects', function (Blueprint $table) {
            $table->unsignedBigInteger('id')->comment('サロゲートキー')->autoIncrement();
            $table->unsignedBigInteger('flight_prohibited_area_object_id')->comment('飛行禁止エリア情報ID');
            $table->string('spatial_id',50)->comment('空間ID（Lv17）');
            $table->string('voxel_bit_file_path',255)->comment('ボクセル地物ビットファイルパス');
            $table->integer('voxel_bit_spatial_zoom_level')->comment('ボクセル地物ローカル空間ズームレベル');
            $table->integer('point_cloud_epsg')->comment('空間座標系');
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
        Schema::dropIfExists('flight_prohibited_area_objects');
    }
};
