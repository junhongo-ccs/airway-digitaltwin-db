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
        Schema::create('data_sources', function (Blueprint $table) {
            $table->Integer('data_source_id')->comment('元情報ID')->primary();
            $table->string('data_create_server_id', 50)->comment('データ作成サーバID');
            $table->string('data_create_project_id', 50)->comment('データ作成プロジェクトID');
            $table->string('data_source',4000)->comment('元データ情報');
            $table->integer('point_cloud_espg')->nullable()->comment('元点群空間座標系');
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
        Schema::dropIfExists('data_sources');
    }
};
