<?php

namespace Database\Seeders;

use Illuminate\Database\Console\Seeds\WithoutModelEvents;
use Illuminate\Database\Seeder;
use App\Models\ObjectMaster;
use Illuminate\Support\Facades\DB;

class ObjectMasterTableSeeder extends Seeder
{
    /**
     * Run the database seeds.
     */
    public function run(): void
    {
        // 全部消す
        DB::table('object_masters')->delete();
        // 開発用のユーザー追加
        ObjectMaster::create(['object_cd'   =>  6,'object_name' => 'Building']);
        ObjectMaster::create(['object_cd'   =>  9,'object_name' => 'Water']);
        ObjectMaster::create(['object_cd'   => 10,'object_name' => 'Rail']);
        ObjectMaster::create(['object_cd'   => 11,'object_name' => 'Road Surface']);
        ObjectMaster::create(['object_cd'   => 14,'object_name' => 'Wire-Condactor']);
        ObjectMaster::create(['object_cd'   => 15,'object_name' => 'Transmisson Tower']);
    }
}
