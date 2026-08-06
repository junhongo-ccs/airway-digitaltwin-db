<?php

namespace App\Console\Commands;

use App\Models\FlightProhibitedAreaObject;
use App\Models\FlightProhibitedAreaObjectMaster;
use Illuminate\Console\Command;

/**
 * junhongo-ccs/airspace 対応（本物の飛行禁止エリア＝国土数値情報DID地区の投入）。
 *
 * POST /airDtw/api/flight_prohibited_area（FlightProhibitedAreaController::
 * set_flight_prohibited_area）はマスタ行の作成後、空間ID紐付け用の外部ネイティブ
 * 実行ファイル（popen('start /B ...')、config('userapi.flightProhibitedExePath')）を
 * 呼び出す設計だが、そのexeは本デプロイに含まれない。Phase Aの注意区域と同様の制約
 * のため、本コマンドでマスタ行と空間ID紐付け行（flight_prohibited_area_objects）を
 * 直接作成し、GET側（AreaObjectController等と同型の単一空間ID完全一致検索）で
 * 取得できるようにする。
 *
 * 入力JSON（junhongo-ccs/airspace側で国土数値情報A16 DID地区GeoJSONから生成）:
 *   flight_prohibited_area_id, name, type_id, range（GeoJSON Polygon）,
 *   representative_point（range内の実在点）, spatial_id（representative_pointの
 *   ズーム17空間ID）, source, epsg
 */
class ImportFlightProhibitedArea extends Command
{
    protected $signature = 'digitaltwin:import-flight-prohibited-area {file}';

    protected $description = '国土数値情報DID地区等、本物の飛行禁止エリアJSONをflight_prohibited_area系テーブルへ投入する';

    public function handle(): int
    {
        $path = $this->argument('file');
        if (!file_exists($path)) {
            $this->error("file not found: {$path}");
            return 1;
        }

        $data = json_decode(file_get_contents($path), true);
        if (!is_array($data)) {
            $this->error('invalid JSON');
            return 1;
        }

        $now = now('UTC');

        $master = FlightProhibitedAreaObjectMaster::where('flight_prohibited_area_id', $data['flight_prohibited_area_id'])->first()
            ?: new FlightProhibitedAreaObjectMaster();
        $master->flight_prohibited_area_id = $data['flight_prohibited_area_id'];
        $master->name = $data['name'];
        $master->from_datetime = $now;
        $master->to_datetime = '9999-12-31 23:59:59';
        $master->range = json_encode($data['range'], JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
        $master->detail = mb_substr($data['source'], 0, 255);
        $master->url = 'https://nlftp.mlit.go.jp/ksj/gml/datalist/KsjTmplt-A16-2020.html';
        $master->flight_prohibited_area_type_id = $data['type_id'];
        $master->status = 1;
        $master->save();

        $object = FlightProhibitedAreaObject::where('flight_prohibited_area_object_id', $master->flight_prohibited_area_object_id)
            ->where('spatial_id', $data['spatial_id'])
            ->first() ?: new FlightProhibitedAreaObject();
        $object->flight_prohibited_area_object_id = $master->flight_prohibited_area_object_id;
        $object->spatial_id = $data['spatial_id'];
        $object->voxel_bit_file_path = "flight_prohibited_area/{$data['flight_prohibited_area_id']}.json";
        $object->voxel_bit_spatial_zoom_level = 17;
        $object->point_cloud_epsg = $data['epsg'];
        $object->save();

        $this->info(
            "registered flight_prohibited_area_id={$data['flight_prohibited_area_id']} "
            . "(master_id={$master->flight_prohibited_area_object_id}, spatial_id={$data['spatial_id']}, "
            . "representative_point=lat{$data['representative_point']['lat']},lon{$data['representative_point']['lon']})"
        );
        return 0;
    }
}
