<?php

namespace Database\Seeders;

use Illuminate\Database\Console\Seeds\WithoutModelEvents;
use Illuminate\Database\Seeder;
use App\Models\DataSource;
use Illuminate\Support\Facades\DB;

class DataSourceTableSeeder extends Seeder
{
    /**
     * Run the database seeds.
     */
    public function run(): void
    {
        // 全部消す
        DB::table('data_sources')->delete();
        // 開発用のユーザー追加
        DataSource::create(['data_source_id' =>  1,'data_create_server_id' => 'local','data_create_project_id' => 'localTest','data_source' => 'ローカルテスト']);
    }
}
