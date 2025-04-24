<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Http\Controllers\Api\ApiFunction;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Log;
use Illuminate\Support\Facades\Storage;
use Illuminate\Support\Facades\File;
use Illuminate\Support\Facades\Auth;
use Illuminate\Support\Facades\DB;
use App\Models\AreaObjectMaster;

class AreaObjectController extends Controller
{   
    public function set_area_object_masters(Request $request)
    {  
       //項目チェック
        try {
            if(json_last_error() !== JSON_ERROR_NONE) {
                throw new \Exception('Parameter error! Json Syntax Err!');
            }
            if(!$request->has('features') ) {
                throw new \Exception('Parameter error! no [features]');
            } else {
                if ($request->features == null) {
                    throw new \Exception('Parameter error! no [features]');               
                }
            }
            //featuresが配列でない時は配列に変換
            $feature_arr = $request["features"];
            if (!(array_values($feature_arr) === $feature_arr)) {
                $feature_arr = array($feature_arr);
            };
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }
        //トランザクション処理開始
        DB::beginTransaction();
        $idArr = [];
        foreach($feature_arr as $feature) {
            try {
            if(!isset($feature["geometry"]["coordinates"]) ) {
                throw new \Exception('Parameter error! no [geometry->coordinates]');
            }
            if(!isset($feature["properties"]["area"]) ) {
                throw new \Exception('Parameter error! no [properties->area]');
            }
            
            if(!isset($feature["properties"]["timestamp"]) ) {
                throw new \Exception('Parameter error! no [properties->timestamp]');
            } else {
                $timestamp = ApiFunction::format_iso8601($feature["properties"]["timestamp"]);
                if (!ApiFunction::check_datetime($timestamp)) {
                    throw new \Exception('Parameter error! timestamp\'s format is wrong ');               
                }
            }  
            if(!isset($feature["properties"]["intrusionStatus"]) ) {
                throw new \Exception('Parameter error! no [properties->intrusionStatus]');
            } else {
                if (!is_numeric($feature["properties"]["intrusionStatus"])) {
                    throw new \Exception('Parameter error! [intrusionStatus]  is not numeric.');
                }
            }
            if(!isset($feature["properties"]["traffics"]) ) {
                throw new \Exception('Parameter error! no [properties->traffics]');
            }
            $fromTime = "9999-12-31 23:59:59";
            foreach($feature["properties"]["traffics"] as $traffic) {
                if(!isset($traffic["currentTime"]) ) {
                    throw new \Exception('Parameter error! no [properties->traffics->currentTime]');
                } else {
                    $currentTime = ApiFunction::format_iso8601($traffic["currentTime"]);
                    if (!ApiFunction::check_datetime($currentTime)) {
                        throw new \Exception('Parameter error! currentTime\'s format is wrong ');               
                    }
                    if ($fromTime > $currentTime) {
                        $fromTime = $currentTime;               
                    }
                }
            }
            } catch (\Throwable $e) {
                DB::rollback();
                return response()->json([  'message' => $e->getMessage()], 400,
                    ['Content-Type' => 'application/json;charset=UTF-8'],
                    JSON_UNESCAPED_UNICODE
                );
            }

            $coordinates_json = json_encode($feature["geometry"]["coordinates"], JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
            $traffics_json = json_encode($feature["properties"]["traffics"], JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
            try {
                $return_create = AreaObjectMaster::create(  
                ["area_id"          => $feature["properties"]["area"],
                 "from_datetime"    => $fromTime,
                 "to_datetime"      => "9999-12-31 23:59:59",
                 "instrusion_status" => $feature["properties"]["intrusionStatus"],
                 "timestamp"        => $timestamp,
                 "coordinates"      => $coordinates_json,
                 "traffics"         => $traffics_json,
                 "status"           => 1 ]
                ); 
                $idArr[] = $return_create->area_object_id;
            } catch (\Throwable $e) {
                DB::rollback();
                return response()->json([  'message' => 'db insert error '.$e->getMessage()], 500,
                    ['Content-Type' => 'application/json;charset=UTF-8'],
                    JSON_UNESCAPED_UNICODE
                );
            } 
        }

        DB::commit();
        $cmd_id = "";
        foreach ($idArr as $index => $id) {
            if ($index === array_key_last($idArr)) {
                $cmd_id = $cmd_id.strval($id); 
            } else {
                $cmd_id = $cmd_id.strval($id).","; 
            }
        }
           
        $cmd = config('userapi.droneAreaExePath').' '.$cmd_id.' '.config('userapi.droneAreaConfigPath');
        $ps_return = popen( 'start /B '.$cmd, 'r' );
        pclose($ps_return);
        return ;      
    }

    public static function get_area($request)  
    {
        //timing_toチェック
        if(!$request->has('other') ) {
            throw new \Exception('Parameter error! no [other]');
        } else {
            if(!isset($request['other']['timingTo'])) {
                throw new \Exception('Parameter error!  no [timingTo].'); 
            }
        }
        $timing_to = $request['other']['timingTo'];
        if (!ApiFunction::check_datetime($timing_to)) {
            throw new \Exception('Parameter error! timingTo\'s format is wrong ');
        }              
    
        $spatial_id = $request["identification"];
        $selectResults = AreaObjectMaster::
        //select('items.name','units.code')
        where([
            ['from_datetime', '>=', $request->timing],
            ['from_datetime',   '<=', $timing_to],
        ])->with('area_spatial')
        ->whereHas('area_spatial', function ($query) use ($spatial_id) {
            return $query->where('spatial_id', "=", $spatial_id);
        })
        ->orderBy('area_id', 'asc')
        ->orderBy('from_datetime', 'asc')->get(); 
        $objects = [];
        if ( $selectResults != false){
            //$wk_area_id = "";
            foreach ( $selectResults as  $selectResult) {
                //if ($wk_area_id != $selectResult->area_id) {
                //    $wk_area_id = $selectResult->area_id;
                    $work = ['spatialId' => $spatial_id];
                    $timing = ['fromDatetime' =>$selectResult->from_datetime]; 
                    if($selectResult->to_datetime == "9999-12-31 23:59:59") {
                        $timing['endDatetime'] = null; 
                    } else {
                        $timing['endDatetime'] = $selectResult->to_datetime;
                    } 
                    $work['timing'] = $timing;
                    $other = ['voxelBitFileName' => config('userapi.addHttpRootPath'). $selectResult["area_spatial"][0]["voxel_bit_file_path"]]; 
                    $other['voxelBitSpatialZoomLevel']  = $selectResult["area_spatial"][0]["voxel_bit_spatial_zoom_level"];
                    $other['voxelBitEpsg']  = $selectResult["area_spatial"][0]["point_cloud_epsg"];
                    $other['area'] = $selectResult->area_id;
                    $other['timestamp'] = $selectResult->timestamp;  
                    $other['intrusionStatus'] = $selectResult->instrusion_status;
                    $traffics_json = json_decode($selectResult->traffics);        
                    $other['traffics']  = $traffics_json;
                    $work['other'] = $other; 
                    $objects[] = $work;          
                //}
            } 
        }
        //レスポンス用のjson配列編集
        $objects_res = ['objects' => $objects];                              
        return  $objects_res;          
    }


}
