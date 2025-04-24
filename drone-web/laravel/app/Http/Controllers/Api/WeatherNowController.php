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
use App\Models\WeatherNow;
use App\Models\WeatherNowSpatial;

class WeatherNowController extends Controller
{   
    public function set_weather_now(Request $request)
    {  
    //項目チェック
    try {
        if(json_last_error() !== JSON_ERROR_NONE) {
            throw new \Exception('Parameter error! Json Syntax Err!');
        }
        if($request->has('ugrd') ) {
            $ugrds = $request["ugrd"];
            WeatherNowController::weatherNow_dataCreate($ugrds,"ugrd");
        }
        if($request->has('vgrd') ) {
            $vgrds = $request["vgrd"];
            WeatherNowController::weatherNow_dataCreate($vgrds,"vgrd");
        } 
        if($request->has('pres') ) {
            $press = $request["pres"];
            WeatherNowController::weatherNow_dataCreate($press,"pres");
        } 
        if($request->has('hgt') ) {
            $hgts = $request["hgt"];
            WeatherNowController::weatherNow_dataCreate($hgts,"hgt");
        } 
    } catch (\Throwable $e) {
        if(str_starts_with($e->getMessage(), "Parameter error!" )){
            return response()->json([ 'message' => $e->getMessage() ], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        } else {
            return response()->json([ 'message' => $e->getMessage() ], 500,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
    } 
        return;
    }

    public static function weatherNow_dataCreate($datas,$element)
    { 
    
        foreach($datas as $data) {
            //$data = $request["data"];
            if(!isset($data['basetime'])) {
                throw new \Exception('Parameter error! no [basetime]');
            } 
            if(!isset($data['validtime'])) {
                throw new \Exception('Parameter error! no [validtime]');
            } else {
                //日時のフォーマットチェック
                $tmpValidTime = str_replace('T', '', $data['validtime']);
                $newValidTime = ApiFunction::edit_datetime($tmpValidTime."00");
                if(!ApiFunction::check_datetime($newValidTime)) {
                    throw new \Exception('Parameter error! [validtime] is wrong.');
                }
            }
            if(!isset($data['name']))  {
                throw new \Exception('Parameter error! no [name]');
            }
            if(!isset($data['contents']))  {
                throw new \Exception('Parameter error! no [contents]');
            }
            if(!isset($data['latStart']))  {
                throw new \Exception('Parameter error! no [latStart]');
            } else {
                if (!is_numeric($data['latStart'])) {
                    throw new \Exception('Parameter error! [latStart is not numeric]');
                }
            }
            if(!isset($data['lonStart']))  {
                throw new \Exception('Parameter error! no [lonStart]');
            } else {
                if (!is_numeric($data['lonStart'])) {
                    throw new \Exception('Parameter error! [lonStart is not numeric]');
                }
            }
            if(!isset($data['latEnd']))  {
                throw new \Exception('Parameter error! no [latEnd]');
            } else {
                if (!is_numeric($data['latEnd'])) {
                    throw new \Exception('Parameter error! [latEnd is not numeric]');
                }
            }
            if(!isset($data['lonEnd']))  {
                throw new \Exception('Parameter error! no [lonEnd]');
            } else {
                if (!is_numeric($data['lonEnd'])) {
                    throw new \Exception('Parameter error! [lonEnd is not numeric]');
                }
            }
           //トランザクション処理開始
           DB::beginTransaction();
           try {
               $contents_json = json_encode($data["contents"], JSON_UNESCAPED_UNICODE);
                   $return_create = WeatherNow::create(  
                       ["met_elements"          => $element,
                       "from_datetime"          => $newValidTime,
                       "to_datetime"            => "9999-12-31 23:59:59",
                       "basetime"               => $data["basetime"],
                       "name"                   => $data["name"],
                       "contents"               => $contents_json,
                       "lat_start"              => $data['latStart'],
                       "lon_start"              => $data['lonStart'],
                       "lat_end"                => $data['latEnd'],
                       "lon_end"                => $data['lonEnd']]
                   );
               $now_id = $return_create->weather_now_id; 
               $nwlon = $data['lonStart'];
               $nwlat = $data['latStart'];
               $selon = $data['lonEnd'];
               $selat = $data['latEnd'];
               $spatialId_arr = ApiFunction::div_area_into_spatialIds($nwlon,$nwlat,$selon,$selat);
               foreach($spatialId_arr as $spatialId) { 
                   $return_create2 = WeatherNowSpatial::create(  
                       ["weather_now_id"      => $now_id,
                        "spatial_id"          => $spatialId]
                   );
               }
               DB::commit();
           } catch (\Throwable $e) {
               DB::rollback();
               throw new \Exception('DB error! '.$e->getMessage());
           } 
        } 
        return;     
    }

    public static function get_weather_now($request)
    
    {
            $spatial_id = $request->identification;
            $selectResults = WeatherNow::where([
                ['from_datetime', '<=', $request->timing],
                ['to_datetime',   '>=', $request->timing],
            ])->with('weather_now_spatial')
            ->whereHas('weather_now_spatial', function ($query) use ($spatial_id) {
                return $query->where('spatial_id', "=", $spatial_id);
            })
            ->orderBy('met_elements', 'asc')
            ->orderBy('from_datetime', 'desc')
            ->orderBy('weather_now_id', 'desc')->get();  
            //->orderBy('mesh5', 'asc')
            //->orderBy('from_datetime', 'desc')->get(); 
            $forecast = [];
            if ( $selectResults != false){
                $old_elm = "";
                $timing['fromDatetime'] = "9999-12-31 23:59:59";
                $timing['endDatetime']   = "0000-01-01 00:00:00";
                foreach ( $selectResults as  $selectResult) {
                    //Log::info("ele=".$selectResult->met_elements);
                    //Log::info("sta=".$selectResult->from_datetime);
                    //Log::info("id=".$selectResult->weather_now_id);
                    if ($old_elm != $selectResult->met_elements) {
                        $old_elm = $selectResult->met_elements;
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
                        $tmp_arr['basetime'] = $selectResult->basetime;
                        $tmp_datetime = str_replace(' ', 'T', $selectResult->from_datetime);
                        $tmp_arr['validtime'] = substr(preg_replace('/[\s\-:]/', '', $tmp_datetime),0,13);
                        $values = json_decode($selectResult->contents);
                        $return_arr = ApiFunction::edit_contens_on_spatial($spatial_id,$values,
                                      $selectResult->lat_start,$selectResult->lon_start,$selectResult->lat_end,$selectResult->lon_end);
                        $tmp_arr['contents'] = $return_arr;
                        $tmp_arr['latStart'] = $selectResult->lat_start;
                        $tmp_arr['lonStart'] = $selectResult->lon_start;
                        $tmp_arr['latEnd'] = $selectResult->lat_end;
                        $tmp_arr['lonEnd'] = $selectResult->lon_end;
                        $tmp_arr['name'] = $selectResult->name;
                        $forecast[$selectResult->met_elements] = $tmp_arr;
                    }
                } 
                /*
                foreach ( $selectResults as  $selectResult) {
                    if ($old_elm != $selectResult->met_elements) {
                        $wk_elm = $selectResult->met_elements;
                        $work = ['spatial_id' => $spatial_id];
                        $timing = ['start_datetime' =>$selectResult->from_datetime]; 
                        $timing['end_datetime'] = $selectResult->to_datetime; 
                        $work['timing'] = $timing; 
                        $tmp_arr = ['mesh5' => $selectResult->mesh5];    
                        $tmp_arr['weather_time']  = $selectResult->from_datetime;
                        $tmp_arr['nowcast_flag'] = $selectResult->nowcast_flag;
                        $tmp_arr['nowcast_value'] = $selectResult->nowcast_value;
                        $forecast[] = $tmp_arr;
                        
                    }
                } */
            }
            //レスポンス用のjson配列編集
            $work [ 'other' ] = $forecast;
            $objects[] = $work;
            $objects_res = ['objects' => $objects];                              
            return  $objects_res;          
    }

}
