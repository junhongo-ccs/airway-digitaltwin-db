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
        Schema::create('object_masters', function (Blueprint $table) {
            $table->Integer('object_cd')->comment('地物タイプコード')->primary();
            $table->string('object_name', 255)->comment('地物タイプ名');
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
        Schema::dropIfExists('object_masters');
    }
};
