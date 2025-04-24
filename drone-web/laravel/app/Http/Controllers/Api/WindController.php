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
use App\Models\Wind;
use App\Models\WindSpatial;

class WindController extends Controller
{   
    public function set_wind(Request $request)
    {  
        /*test
        $lon = 110744 ;
        $lat = 56398 ;
        $crs = '4326';
        $zoom = 17;
        $spatialId = "17/0/".strval($lon)."/".strval($lat);
        $spatialId = "17/0/110744/56398";
        $srat_time = microtime(true);
        //for ($i=0; $i<1000; $i++) {
        $returnC = shell_exec( config('userapi.exepath') . ' 1 ' . $spatialId . ' ' . $crs);
        //return $returnC;
        $points_arr = explode(" ", $returnC);
        //}
        $end_timeC = microtime(true);
        //for ($i=0; $i<1000; $i++) {
        $returnPHP = ApiFunction::get_vertex_on_voxel($lon, $lat, $zoom);
        //}
        $end_timePHP = microtime(true);
        $timeC = $end_timeC - $srat_time;
        $timePHP = $end_timePHP - $end_timeC;
        $returnC_long = $points_arr[9] . ":" . $points_arr[10]. ":" .$points_arr[0] . ":" . $points_arr[7];
        $returnPHP_long = strval($returnPHP[0]) . ":" . strval($returnPHP[1]) . ":" . strval($returnPHP[2]) . ":" . strval($returnPHP[3]);
        //return strval($returnC) . "  " . strval($returnPHP);
        return $returnC_long . " \n" . $returnPHP_long;
        //
        /*test
        $lon = 124.16875 ;
        $lat = 24.33125;
        $crs = '4326';
        $zoom = '17';
        $repPoints = $lat.":".$lon.":"."0.0";
        $srat_time = microtime(true);
        //for ($i=0; $i<1000; $i++) {
        $returnC = shell_exec( config('userapi.exepath') . ' 3 ' . $repPoints  . ' ' . $crs . ' ' . $zoom);
        //}
        $end_timeC = microtime(true);
        //for ($i=0; $i<1000; $i++) {
        $returnPHP = ApiFunction::get_spatial_xy_on_point($lon, $lat, $zoom);
        //}
        $end_timePHP = microtime(true);
        $timeC = $end_timeC - $srat_time;
        $timePHP = $end_timePHP - $end_timeC;
        return strval($returnC) . "  " . strval($returnPHP);
        */
        //項目チェック
        try {
            /*test
            $mesh5 = 3624319321;
            return ApiFunction::div_area_into_spatialIds($mesh5);
            */
            /*test
            $mesh5 = 3624410321;
            return ApiFunction::div_area_into_spatialIds($mesh5);
            */
            if(json_last_error() !== JSON_ERROR_NONE) {
                throw new \Exception('Parameter error! Json Syntax Err!');
            }
            if(!$request->has('weather') ) {
                throw new \Exception('Parameter error! no [weather]');
            } else {
                if ($request->weather == null) {
                    throw new \Exception('Parameter error! no [weather]');
                }
            }
            $weathers = $request["weather"];
            //dataが配列でない時は配列に変換
            if (!(array_values($weathers) === $weathers)) {
                $weathers = array($weathers);
            };
            //datasの要素数だけDB登録を実行
            foreach($weathers as $weather) {
                //$data = $request["data"];
                if(!isset($weather['type'])) {
                    throw new \Exception('Parameter error! no [type]');
                } else {
                    if ($weather['type'] == "") {
                        throw new \Exception('Parameter error! no [type]');               
                    }
                }
                if(!isset($weather['timeStart'])) {
                    throw new \Exception('Parameter error! no [timeStart]');
                } else {
                //日時のフォーマットチェック
                    $newStartTime = ApiFunction::edit_datetime($weather['timeStart']);
                    if(!ApiFunction::check_datetime($newStartTime)) {
                        throw new \Exception('Parameter error! [timeStart] is wrong.');
                    }
                }
                $newFinishTime = "9999-12-31 23:59:59";
                if(isset($weather["timeEnd"]) ) {
                    $tmpFinishTime = ApiFunction::edit_datetime($weather['timeEnd']);
                    if (ApiFunction::check_datetime($tmpFinishTime)) {
                        $newFinishTime = $tmpFinishTime;             
                    }
                }

                if(!isset($weather['lonStart']))  {
                    throw new \Exception('Parameter error! no [lonStart]');
                } else {
                    if (!is_numeric($weather['lonStart'])) {
                        throw new \Exception('Parameter error! [lonStart] is not numeric');
                    }
                }

                if(!isset($weather['latStart']))  {
                    throw new \Exception('Parameter error! no [latStart]');
                } else {
                    if (!is_numeric($weather['latStart'])) {
                        throw new \Exception('Parameter error! [latStart] is not numeric');
                    }
                }

                if(!isset($weather['lonInterval']))  {
                    throw new \Exception('Parameter error! no [lonInterval]');
                } else {
                    if (!is_numeric($weather['lonInterval'])) {
                        throw new \Exception('Parameter error! [lonInterval] is not numeric');
                    }
                }

                if(!isset($weather['latInterval']))  {
                    throw new \Exception('Parameter error! no [latInterval]');
                } else {
                    if (!is_numeric($weather['latInterval'])) {
                        throw new \Exception('Parameter error! [latInterval] is not numeric');
                    }
                }

                if(!isset($weather['lonCount']))  {
                    throw new \Exception('Parameter error! no [lonCount]');
                } else {
                    if (!is_numeric($weather['lonCount'])) {
                        throw new \Exception('Parameter error! [lonCount] is not numeric');
                    }
                }
               
                if(!isset($weather['values'])) {
                    throw new \Exception('Parameter error! no [values]');
                } 
                //トランザクション処理開始
                DB::beginTransaction();
                try {
                    $values_json = json_encode($weather["values"], JSON_UNESCAPED_UNICODE);
                        $return_create = Wind::create(  
                            ["type"            => $weather['type'],
                            "from_datetime"    => $newStartTime,
                            "to_datetime"      => $newFinishTime,
                            "lon_start"        => $weather['lonStart'],
                            "lat_start"        => $weather['latStart'],
                            "lon_interval"     => $weather['lonInterval'],
                            "lat_interval"     => $weather['latInterval'],
                            "lon_count"        => $weather['lonCount'],
                            "lat_count"        => $weather['latCount'],
                            "values"           => $values_json ]
                        );
                    $wind_id = $return_create->wind_id; 
                    $nwlon = $weather['lonStart'];
                    $nwlat = $weather['latStart'];
                    $selon = $nwlon + $weather['lonInterval'] * $weather['lonCount'];
                    $selat = $nwlat + $weather['latInterval'] * $weather['latCount'];
                    $spatialId_arr = ApiFunction::div_area_into_spatialIds($nwlon,$nwlat,$selon,$selat);
                    //Log::info("nwLon="+strVal($nwlon));
                    //Log::info("nwLat="+strVal($nwlat));
                    //Log::info("seLon="+strVal($selon));
                    //Log::info("seLat="+strVal($selat));
                    foreach($spatialId_arr as $spatialId) { 
                        $return_create2 = WindSpatial::create(  
                            ["wind_id"             => $wind_id,
                             "spatial_id"          => $spatialId]
                        );
                    }
                    DB::commit();
                } catch (\Throwable $e) {
                    DB::rollback();
                    return response()->json([  'message' => 'db update error '.$e->getMessage()], 500,
                    ['Content-Type' => 'application/json;charset=UTF-8'],
                    JSON_UNESCAPED_UNICODE
                    );
                } 
            }
                  
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }
        
        return ;
    }

    public static function set_wind_spatial(Request $request)
    {  
        /*test
        $lon = 110744 ;
        $lat = 56398 ;
        $crs = '4326';
        $zoom = 17;
        $spatialId = "17/0/".strval($lon)."/".strval($lat);
        $spatialId = "17/0/110744/56398";
        $srat_time = microtime(true);
        //for ($i=0; $i<1000; $i++) {
        $returnC = shell_exec( config('userapi.exepath') . ' 1 ' . $spatialId . ' ' . $crs);
        //return $returnC;
        $points_arr = explode(" ", $returnC);
        //}
        $end_timeC = microtime(true);
        //for ($i=0; $i<1000; $i++) {
        $returnPHP = ApiFunction::get_vertex_on_voxel($lon, $lat, $zoom);
        //}
        $end_timePHP = microtime(true);
        $timeC = $end_timeC - $srat_time;
        $timePHP = $end_timePHP - $end_timeC;
        $returnC_long = $points_arr[9] . ":" . $points_arr[10]. ":" .$points_arr[0] . ":" . $points_arr[7];
        $returnPHP_long = strval($returnPHP[0]) . ":" . strval($returnPHP[1]) . ":" . strval($returnPHP[2]) . ":" . strval($returnPHP[3]);
        //return strval($returnC) . "  " . strval($returnPHP);
        return $returnC_long . " \n" . $returnPHP_long;
        //
        /*test
        $lon = 124.16875 ;
        $lat = 24.33125;
        $crs = '4326';
        $zoom = '17';
        $repPoints = $lat.":".$lon.":"."0.0";
        $srat_time = microtime(true);
        //for ($i=0; $i<1000; $i++) {
        $returnC = shell_exec( config('userapi.exepath') . ' 3 ' . $repPoints  . ' ' . $crs . ' ' . $zoom);
        //}
        $end_timeC = microtime(true);
        //for ($i=0; $i<1000; $i++) {
        $returnPHP = ApiFunction::get_spatial_xy_on_point($lon, $lat, $zoom);
        //}
        $end_timePHP = microtime(true);
        $timeC = $end_timeC - $srat_time;
        $timePHP = $end_timePHP - $end_timeC;
        return strval($returnC) . "  " . strval($returnPHP);
        */
        //項目チェック
        try {
            if(json_last_error() !== JSON_ERROR_NONE) {
                throw new \Exception('Parameter error! Json Syntax Err!');
            }
            if(!$request->has('data') ) {
                throw new \Exception('Parameter error! no [data]');
            } else {
                if ($request->data == null) {
                    throw new \Exception('Parameter error! no [data]');
                }
            }
            $datas = $request["data"];
            //dataが配列でない時は配列に変換
            if (!(array_values($datas) === $datas)) {
                $datas = array($datas);
            };
            //datasの要素数だけDB登録を実行
            foreach($datas as $data) {
                //$data = $request["data"];
                if(!isset($data['mesh5'])) {
                    throw new \Exception('Parameter error! no [mesh5]');
                } else {
                    if ($data['mesh5'] == "") {
                        throw new \Exception('Parameter error! no [mesh5]');               
                    }
                }

                if(!isset($data['elevation_min']))  {
                    throw new \Exception('Parameter error! no [elevation_min]');
                } else {
                    if (!is_numeric($data['elevation_min'])) {
                        throw new \Exception('Parameter error! [elevation_min is not numeric]');
                    }
                }
                if(!isset($data['elevation_max']))  {
                    throw new \Exception('Parameter error! no [elevation_max]');
                } else {
                    if (!is_numeric($data['elevation_max'])) {
                        throw new \Exception('Parameter error! [elevation_max is not numeric]');
                    }
                }
                if(!isset($data['weather_time'])) {
                    throw new \Exception('Parameter error! no [weather_time]');
                } else {
                //日時のフォーマットチェック 
                    if(!ApiFunction::check_datetime($data['weather_time'])) {
                        throw new \Exception('Parameter error! [weather_time] is wrong.');
                    }
                }
                if(!isset($data['forecast'])) {
                    throw new \Exception('Parameter error! no [weather_time]');
                } 
                //トランザクション処理開始
                DB::beginTransaction();
                try {
                    $forecast_json = json_encode($data["forecast"], JSON_UNESCAPED_UNICODE);
                    $return_create = Wind::create(  
                        ["mesh5"              => $data["mesh5"],
                         "elevation_min"      => $data["elevation_min"],
                         "elevation_max"      => $data["elevation_max"],
                         "from_datetime"      => $data["weather_time"],
                         "to_datetime"        => "9999-12-31 23:59:59",
                         "forecast"           => $forecast_json ]
                        );  
                DB::commit();
                } catch (\Throwable $e) {
                    DB::rollback();
                    return response()->json([  'message' => 'db update error '.$e->getMessage()], 500,
                    ['Content-Type' => 'application/json;charset=UTF-8'],
                    JSON_UNESCAPED_UNICODE
                    );
                } 
            }
                  
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }
        
        return;
    }

    public static function get_wind($request)  
    {
        //ドローン空路名
        // 検索
        //DroneRouteSpatialからdrone_route_id検索
        $spatial_id = $request->identification;
        $selectResults = Wind::where([
            ['from_datetime', '<=', $request->timing],
            ['to_datetime',   '>=', $request->timing],
        ])->with('wind_spatial')
        ->whereHas('wind_spatial', function ($query) use ($spatial_id) {
            return $query->where('spatial_id', "=", $spatial_id);
        })
        ->orderBy('type', 'asc')
        ->orderBy('from_datetime', 'desc')
        ->orderBy('wind_id', 'desc')->get();  
        /*->orderBy([
            'type' => 'asc',
            'from_datetime' => 'desc',
            'wind_id' => 'desc'
        ])->get();
        */
       // return json_encode($selectResults);
        $weather = [];
        if ( $selectResults != false){
            $old_type = "";
            $timing['fromDatetime'] = "9999-12-31 23:59:59";
            $timing['endDatetime']   = "0000-01-01 00:00:00";
            foreach ( $selectResults as  $selectResult) {
                if ($old_type != $selectResult->type) {
                    $old_type = $selectResult->type;
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
                    $tmp_arr = ['type' => $selectResult->type];    
                    $tmp_arr['timeStart'] = preg_replace('/[\s\-:]/', '', $selectResult->from_datetime);
                    if($selectResult->to_datetime == "9999-12-31 23:59:59") {
                        $tmp_arr['timeEnd'] = null; 
                    } else {
                        $tmp_arr['timeEnd'] = preg_replace('/[\s\-:]/', '', $selectResult->to_datetime);
                    }
                    $tmp_arr['lonStart'] = round($selectResult->lon_start,6);
                    $tmp_arr['latStart'] = round($selectResult->lat_start,6);
                    $tmp_arr['lonInterval'] = round($selectResult->lon_interval,6);
                    $tmp_arr['latInterval'] = round($selectResult->lat_interval,6);
                    $tmp_arr['lonCount']  = $selectResult->lon_count;
                    $tmp_arr['latCount']  = $selectResult->lat_count;
                    //$tmp_arr['values']  = $selectResult->values;
                    $values = json_decode($selectResult->values);
                    $array2D = array_chunk($values, $selectResult->lon_count);
                    $lat_end = $selectResult->lat_start + $selectResult->lat_interval * $selectResult->lat_count;
                    $lon_end = $selectResult->lon_start + $selectResult->lon_interval * $selectResult->lon_count;
                    //$array2D = array_chunk($selectResult->values, $selectResult->lon_count);
                    $return_arr = ApiFunction::edit_contens_on_spatial($spatial_id,$array2D,$selectResult->lat_start,$selectResult->lon_start,$lat_end,$lon_end);
                    //$tmp_arr['values'] = array_reduce($return_arr, 'array_merge', []);
                    $tmp_arr['values'] = array_merge(...$return_arr);
                    $weather[] = $tmp_arr;
                }
            } 
        }
        //レスポンス用のjson配列編集
        $src = [ 'weather' => $weather ];
        $work [ 'other' ] = $src;
        $objects[] = $work;
        $objects_res = ['objects' => $objects];                              
        return  $objects_res;          
    }

}
  
/*public static function get_wind($request)  
    {
        //ドローン空路名検索
        //DroneRouteSpatialからdrone_route_id検索
        $spatial_id = $request->identification;
        $selectResults = Wind::where([
            ['from_datetime', '<=', $request->timing],
            ['to_datetime',   '>=', $request->timing],
        ])->with('wind_spatial')
        ->whereHas('wind_spatial', function ($query) use ($spatial_id) {
            return $query->where('spatial_id', "=", $spatial_id);
        })
        ->orderBy('mesh5', 'asc')
        ->orderBy('from_datetime', 'desc')->get(); 
        $forecast = [];
        if ( $selectResults != false){
            $wk_mesh5 = "";
            foreach ( $selectResults as  $selectResult) {
                if ($wk_mesh5 != $selectResult->mesh5) {
                $work = ['spatial_id' => $spatial_id];
                $timing = ['start_datetime' =>$selectResult->from_datetime]; 
                $timing['end_datetime'] = $selectResult->to_datetime; 
                $work['timing'] = $timing; 
                $tmp_arr = ['mesh5' => $selectResult->mesh5];    
                $tmp_arr['elevation_min'] = $selectResult->elevation_min;
                $tmp_arr['elavation_max'] = $selectResult->elevation_max;
                $tmp_arr['weather_time']  = $selectResult->from_datetime;
                $tmp_arr['forecast']  = $selectResult->forecast;
                $forecast[] = $tmp_arr;
                $wk_mesh5 = $selectResult->mesh5;
                }
            } 
        }
        //レスポンス用のjson配列編集
        $src = [ 'src' => $forecast ];
        $work [ 'other' ] = $src;
         $objects_res = ['objects' => $work];                              
        return  $objects_res;          
    }

}*/
