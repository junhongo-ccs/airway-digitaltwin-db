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
use App\Models\FlightProhibitedAreaObjectMaster;
use App\Models\FlightProhibitedAreaObject;

class FlightProhibitedAreaController extends Controller
{   
    public function set_flight_prohibited_area(Request $request)
    {  
        //項目チェック
        try {
            if(json_last_error() !== JSON_ERROR_NONE) {
                throw new \Exception('Parameter error! Json Syntax Err!');
            }
            if(!$request->has('flightProhibitedAreaInfo') ) {
                throw new \Exception('Parameter error! no [flightProhibitedAreaInfo]');
            } else {
                if ($request->flightProhibitedAreaInfo == null) {
                    throw new \Exception('Parameter error! no [flightProhibitedAreaInfo]');               
                }
            }
            //flightProhibitedAreaInfoが配列でない時は配列に変換
            $area_arr = $request["flightProhibitedAreaInfo"];
            if (!(array_values($area_arr) === $area_arr)) {
                $area_arr = array($area_arr);
            };
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }
            //flightProhibitedAreaInfo配列分のloop
        DB::beginTransaction();
            $idArr = [];
            foreach($area_arr as $area) {
                try {
                //項目チェック
                if (!isset($area["flightProhibitedAreaId"])) {
                    throw new \Exception('Parameter error! no [flightProhibitedAreaId]'); 
                } else {
                    if ($area["flightProhibitedAreaId"] == "") {
                        throw new \Exception('Parameter error! no [flightProhibitedAreaId]'); 
                    }
                }
                if (!isset($area["name"])) {
                    throw new \Exception('Parameter error! no [name]'); 
                } else {
                    if ($area["name"] == "") {
                        throw new \Exception('Parameter error! no [name]'); 
                    }
                }
                if (!isset($area["range"])) {
                    throw new \Exception('Parameter error! no [range]'); 
                } else {
                    if ($area["range"] == "") {
                        throw new \Exception('Parameter error! no [range]'); 
                    }
                }
                if (!array_key_exists("detail",$area)) {
                    $area["detail"] = null;
                }
                if (!array_key_exists("url",$area)) {
                    $area["url"] = null;
                }
                if (!isset($area["flightProhibitedAreaTypeId"])) {
                    throw new \Exception('Parameter error! no [flightProhibitedAreaTypeId]');
                } else {
                    if (!is_numeric($area["flightProhibitedAreaTypeId"])) {
                        throw new \Exception('Parameter error! [flightProhibitedAreaTypeId]  is not numeric.');
                    }
                }
                if(!isset($area["startTime"]) ) {
                    throw new \Exception('Parameter error! no [startTime]');
                } else {
                    $newStartTime = str_replace(' ', '', $area["startTime"]);
                    $newStartTime = ApiFunction::edit_datetime($newStartTime."00");
                    if (!ApiFunction::check_datetime($newStartTime)) {
                        throw new \Exception('Parameter error! startTime\'s format is wrong ');               
                    }
                }
                $newFinishTime = "9999-12-31 23:59:59";
                    if(isset($area["finishTime"]) ) {
                        $tmpFinishTime = str_replace(' ', '', $area["finishTime"]);
                        $tmpFinishTime = ApiFunction::edit_datetime($tmpFinishTime);
                        if (ApiFunction::check_datetime($tmpFinishTime)) {
                            $newFinishTime = $tmpFinishTime;             
                        }
                    }
                $range_json = json_encode($area["range"], JSON_UNESCAPED_UNICODE );

                } catch (\Throwable $e) {
                    DB::rollback();
                    return response()->json([  'message' => $e->getMessage()], 400,
                        ['Content-Type' => 'application/json;charset=UTF-8'],
                        JSON_UNESCAPED_UNICODE
                    );
                }

                try {
                    $return_create = FlightProhibitedAreaObjectMaster::Create(  
                    ["flight_prohibited_area_id"     => $area["flightProhibitedAreaId"],
                    "name"             => $area["name"],
                    "from_datetime"    => $newStartTime,
                    "to_datetime"      => $newFinishTime,
                    "range"            => $range_json,
                    "detail"           => $area["detail"],
                    "url"              => $area["url"],
                    "flight_prohibited_area_type_id"     => $area["flightProhibitedAreaTypeId"],
                    "status"           => 1 ]
                    );  
                    $idArr[] = $return_create->flight_prohibited_area_object_id;
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
            $cmd = config('userapi.flightProhibitedExePath').' '.$cmd_id.' '.config('userapi.flightProhibitedConfigPath');
            $ps_return = popen( 'start /B '.$cmd, 'r' );
            pclose($ps_return);
            return ;
    }

    public static function get_flight_prohibited_area($request)  
    {

        $spatial_id = $request->identification;
        $selectResults = FlightProhibitedAreaObjectMaster::
        //select('items.name','units.code')
        where([
            ['from_datetime', '<=', $request->timing],
            ['to_datetime',   '>=', $request->timing],
        ])->with('flight_prohibited_area_object')
        ->whereHas('flight_prohibited_area_object', function ($query) use ($spatial_id) {
            return $query->where('spatial_id', "=", $spatial_id);
        })
        ->orderBy('flight_prohibited_area_id', 'asc')
        ->orderBy('from_datetime', 'desc')
        ->orderBy('flight_prohibited_area_object_id', 'desc')->get(); 
        $objects = [];
        if ( $selectResults != false){
            $wk_id = "";
            $timing['fromDatetime'] = "9999-12-31 23:59:59";
            $timing['endDatetime']   = "0000-01-01 00:00:00";
            foreach ( $selectResults as  $selectResult) {
                if ($wk_id != $selectResult->flight_prohibited_area_id) {
                    $wk_id = $selectResult->flight_prohibited_area_id;
                    $work = ['spatialId' => $spatial_id];
                    if($timing['fromDatetime'] >=  $selectResult->from_datetime) {
                        $timing['fromDatetime'] = $selectResult->from_datetime; 
                    }
                    if($timing['endDatetime'] != null) {
                        if($selectResult->to_datetime == "9999-12-31 23:59:59") {
                            $timing['endDatetime'] = null; 
                        } else {
                            if($timing['endDatetime']  <=  $selectResult->to_datetime) {
                               $timing['endDatetime']   =  $selectResult->to_datetime;
                            }
                        } 
                    } 
                    $work['timing'] = $timing; 
                    $other = ['voxelBitFileName' => config('userapi.addHttpRootPath'). $selectResult["flight_prohibited_area_object"][0]["voxel_bit_file_path"]];
                    $other['voxelBitSpatialZoomLevel']  = $selectResult["flight_prohibited_area_object"][0]["voxel_bit_spatial_zoom_level"];
                    $other['voxelBitEpsg']  = $selectResult["flight_prohibited_area_object"][0]["point_cloud_epsg"];
                    $other['flightProhibitedAreaId'] = $selectResult->flight_prohibited_area_id;    
                    $other['name'] = $selectResult->name;                  
                    $other['detail']  = $selectResult->detail;
                    $other['url']  = $selectResult->url;
                    $other['flightProhibitedAreaTypeId']  = $selectResult->flight_prohibited_area_type_id;
                    $work['other'] = $other; 
                    $objects[] = $work;          
                }
            } 
        }
        //レスポンス用のjson配列編集
        $objects_res = ['objects' => $objects];                              
        return  $objects_res;          
    }


}
