<?php

namespace App\Console\Commands;

use App\Models\DataSource;
use App\Models\GroundFeatureObject;
use Illuminate\Console\Command;

/**
 * junhongo-ccs/airspace Phase B対応（仕様書§7-2）。ImportPlateauBuildingsの汎用版。
 *
 * 道路・土砂災害・洪水浸水等、footprint/centroid/spatial_idを持つ汎用JSON
 * （junhongo-ccs/airspace側のparse_layer.py出力）をground_feature_objectsへ登録する。
 *
 * object_cdの割り当て（本PoCでの暫定規約。実コードに定義が無いため独自に定める）:
 *   1 = 建物（ImportPlateauBuildingsで登録済み）
 *   2 = 道路
 *   3 = 土砂災害警戒区域
 *   4 = 洪水浸水想定区域
 */
class ImportPlateauFeatures extends Command
{
    protected $signature = 'plateau:import-features {file} {--object-cd=} {--data-source-id=} {--label=}';

    protected $description = 'PLATEAU由来の汎用地物（道路・土砂災害・洪水浸水等）JSONをground_feature_objectsへ投入する';

    public function handle(): int
    {
        $path = $this->argument('file');
        if (!file_exists($path)) {
            $this->error("file not found: {$path}");
            return 1;
        }

        $items = json_decode(file_get_contents($path), true);
        if (!is_array($items) || count($items) === 0) {
            $this->error('invalid or empty JSON');
            return 1;
        }

        $objectCd = (int) $this->option('object-cd');
        $dataSourceId = (int) $this->option('data-source-id');
        $label = $this->option('label') ?: 'unknown';
        $meshCode = $items[0]['mesh_code'] ?? 'unknown';

        $source = DataSource::find($dataSourceId) ?: new DataSource();
        $source->data_source_id = $dataSourceId;
        $source->data_create_server_id = 'poc-local';
        $source->data_create_project_id = 'POC-CHICHIBU';
        $source->data_source = "PLATEAU秩父市2025 {$label}（メッシュ{$meshCode}） "
            . 'https://www.geospatial.jp/ckan/dataset/plateau-11207-chichibu-shi-2025 取得日2026-08-06';
        $source->point_cloud_espg = 6697;
        $source->save();

        $count = 0;
        foreach ($items as $it) {
            $memo = sprintf(
                'POC-CHICHIBU %s centroid=%.6f,%.6f featureId=%s',
                $label,
                $it['centroid_lat'],
                $it['centroid_lon'],
                $it['feature_id']
            );
            $obj = new GroundFeatureObject();
            $obj->spatial_id = $it['spatial_id'];
            $obj->from_datetime = now('UTC');
            $obj->object_cd = $objectCd;
            $obj->voxel_bit_file_path = "plateau/{$label}/{$it['feature_id']}.json";
            $obj->voxel_bit_spatial_zoom_level = 17;
            $obj->point_cloud_epsg = 6697;
            $obj->data_source_id = $dataSourceId;
            $obj->update_memo = mb_substr($memo, 0, 1000);
            $obj->save();
            $count++;
        }

        $this->info("registered {$count} {$label} features (mesh {$meshCode}, object_cd {$objectCd}, data_source_id {$dataSourceId})");
        return 0;
    }
}
