<?php

namespace App\Console\Commands;

use App\Models\DataSource;
use App\Models\GroundFeatureObject;
use Illuminate\Console\Command;

/**
 * junhongo-ccs/airspace Phase B対応（仕様書§7-2）。
 *
 * SpaceInfra-cppはWindows専用のVisual Studioプロジェクトで本デプロイには含まれておらず、
 * かつLaravel側にも地物ボクセルの登録API自体が存在しない（GroundFeatureVoxelController等は
 * 取得専用）。そのため、PLATEAU CityGMLから抽出した建物（LOD0フットプリント＋LOD1高さ）を
 * scripts/parse_bldg.py（junhongo-ccs/airspace側）でJSONへ変換し、本コマンドで
 * ground_feature_objectsへ直接登録する。
 */
class ImportPlateauBuildings extends Command
{
    protected $signature = 'plateau:import-buildings {file} {--data-source-id=1} {--mesh=}';

    protected $description = 'PLATEAU CityGML由来の建物JSONをground_feature_objectsへ投入する（Phase B、SpaceInfra-cpp代替）';

    public function handle(): int
    {
        $path = $this->argument('file');
        if (!file_exists($path)) {
            $this->error("file not found: {$path}");
            return 1;
        }

        $buildings = json_decode(file_get_contents($path), true);
        if (!is_array($buildings)) {
            $this->error('invalid JSON');
            return 1;
        }

        $dataSourceId = (int) $this->option('data-source-id');
        $meshCode = $this->option('mesh') ?: ($buildings[0]['mesh_code'] ?? 'unknown');

        $source = DataSource::find($dataSourceId) ?: new DataSource();
        $source->data_source_id = $dataSourceId;
        $source->data_create_server_id = 'poc-local';
        $source->data_create_project_id = 'POC-CHICHIBU';
        $source->data_source = "PLATEAU秩父市2025 建築物モデル（LOD0/LOD1） 3次メッシュ{$meshCode} "
            . 'https://www.geospatial.jp/ckan/dataset/plateau-11207-chichibu-shi-2025 取得日2026-08-06';
        $source->point_cloud_espg = 6697;
        $source->save();

        $count = 0;
        foreach ($buildings as $b) {
            $memo = sprintf(
                'POC-CHICHIBU height=%.1fm centroid=%.6f,%.6f buildingId=%s',
                $b['height_m'],
                $b['centroid_lat'],
                $b['centroid_lon'],
                $b['building_id']
            );
            $obj = new GroundFeatureObject();
            $obj->spatial_id = $b['spatial_id'];
            $obj->from_datetime = now();
            // object_cd=1（建物）は実コードに定義が無いため本PoCでの暫定割り当て。
            // Streamlit側（viewer/src/api_client.py）も同じ値を照会に使う。
            $obj->object_cd = 1;
            $obj->voxel_bit_file_path = "plateau/{$meshCode}/{$b['building_id']}.json";
            $obj->voxel_bit_spatial_zoom_level = 17;
            $obj->point_cloud_epsg = 6697;
            $obj->data_source_id = $dataSourceId;
            $obj->update_memo = mb_substr($memo, 0, 1000);
            $obj->save();
            $count++;
        }

        $this->info("registered {$count} buildings (mesh {$meshCode}, data_source_id {$dataSourceId})");
        return 0;
    }
}
