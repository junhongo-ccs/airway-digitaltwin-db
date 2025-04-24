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
        Schema::create('drone_route', function (Blueprint $table) {
            $table->unsignedBigInteger('drone_route_info_id')->comment('ドローン航路情報ID')->autoIncrement();
            $table->Integer('drone_route_id')->comment('ドローン航路ID');
            $table->string('drone_route_name',100)->comment('ドローン航路名');
            $table->dateTime('from_datetime')->comment('開始日時');
            $table->dateTime('to_datetime')->nullable()->comment('終了日時');
            $table->text('drone_polyline')->comment('ウェイポイント');
            $table->Integer('status')->comment('処理ステータス');
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
        Schema::dropIfExists('drone_route');
    }
};
