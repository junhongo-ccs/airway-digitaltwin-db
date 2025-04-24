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
use App\Models\WeatherForecast;
use App\Models\WeatherForecastSpatial;

class WeatherForecastController extends Controller
{   
    public function set_weather_forecast(Request $request)
    {  
        //項目チェック
    try {
        if(json_last_error() !== JSON_ERROR_NONE) {
            throw new \Exception('Parameter error! Json Syntax Err!');
        }
        if($request->has('ugrd') ) {
            $datas = $request["ugrd"];
            $type = "ugrd";
            WeatherForecastController::weatherForecast_dataCreate($datas,$type);
        }
        if($request->has('vgrd') ) {
            $datas = $request["vgrd"];
            $type = "vgrd";
            WeatherForecastController::weatherForecast_dataCreate($datas,$type);
        } 
        if($request->has('tmp') ) {
            $datas = $request["tmp"];
            $type = "tmp";
            WeatherForecastController::weatherForecast_dataCreate($datas,$type);
        } 
        if($request->has('apcp') ) {
            $datas = $request["apcp"];
            $type = "apcp";
            WeatherForecastController::weatherForecast_dataCreate($datas,$type);
        } 
        if($request->has('lcdc') ) {
            $datas = $request["lcdc"];
            $type = "lcdc";
            WeatherForecastController::weatherForecast_dataCreate($datas,$type);
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

    public static function weatherForecast_dataCreate($datas,$element)
    { 
        foreach($datas as $data) {
            if(!isset($data['basetime'])) {
                throw new \Exception('Parameter error! no [basetime]');
            } 
            if(!isset($data['validtime'])) {
                throw new \Exception('Parameter error! no [validtime]');
            } else {
                //日時のフォーマットチェック
                $tmpValidTime = str_replace('T', '', $data['validtime']);
                $newValidTime = ApiFunction::edit_datetime($tmpValidTime."0000");
                if(!ApiFunction::check_datetime($newValidTime)) {
                    throw new \Exception('Parameter error! [validtime] is wrong.');
                }
            }
            if(!isset($data['minute']))  {
                throw new \Exception('Parameter error! no [minute]');
            } else {
                if (!is_numeric($data['minute'])) {
                    throw new \Exception('Parameter error! [minute is not numeric]');
                }
            }
            $fromDateTime =  ApiFunction::edit_datetime($tmpValidTime.$data['minute']."00");
            if(!isset($data['contents']))  {
                throw new \Exception('Parameter error! no [contents]');
            }
            //return $endTime;
            if(!isset($data['latInterval']))  {
                throw new \Exception('Parameter error! no [latInterval]');
            } else {
                if (!is_numeric($data['latInterval'])) {
                    throw new \Exception('Parameter error! [latInterval is not numeric]');
                }
            }
            if(!isset($data['lonInterval']))  {
                throw new \Exception('Parameter error! no [lonInterval]');
            } else {
                if (!is_numeric($data['lonInterval'])) {
                    throw new \Exception('Parameter error! [lonInterval is not numeric]');
                }
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
            if(!isset($data['grid']))  {
                throw new \Exception('Parameter error! no [grid]');
            } else {
                if (!is_numeric($data['grid'])) {
                    throw new \Exception('Parameter error! [grid is not numeric]');
                }
            }
           //トランザクション処理開始
           DB::beginTransaction();
           try {
               $contents_json = json_encode($data["contents"], JSON_UNESCAPED_UNICODE);
                $return_create = WeatherForecast::create(  
                   ["met_elements"          => $element,
                   "from_datetime"          => $fromDateTime,
                   "to_datetime"            => "9999-12-31 23:59:59",
                   "basetime"               => $data["basetime"],
                   "minute"                 => $data["minute"],
                   "contents"               => $contents_json,
                   "lat_interval"           => $data['latInterval'],
                   "lon_interval"           => $data['lonInterval'],
                   "lat_start"              => $data['latStart'],
                   "lon_start"              => $data['lonStart'],
                   "lat_end"                => $data['latEnd'],
                   "lon_end"                => $data['lonEnd'],
                   "mesh"                   => $data['grid']]
                );
               $now_id = $return_create->weather_forecast_id; 
               $nwlon = $data['lonStart'];
               $nwlat = $data['latStart'];
               $selon = $data['lonEnd'];
               $selat = $data['latEnd'];
               $spatialId_arr = ApiFunction::div_area_into_spatialIds($nwlon,$nwlat,$selon,$selat);
               foreach($spatialId_arr as $spatialId) { 
                   $return_create2 = WeatherForecastSpatial::create(  
                       ["weather_forecast_id"   => $now_id,
                        "spatial_id"            => $spatialId]
                   );
               }
               DB::commit();
           } catch (\Throwable $e) {
               DB::rollback();
               throw new \Exception('DB error! '.$e->getMessage());
           } 
        } 
        //return;     
    }

    public static function get_weather_forecast($request)
    
    {
            $spatial_id = $request->identification;
            $selectResults = WeatherForecast::where([
                ['from_datetime', '<=', $request->timing],
                ['to_datetime',   '>=', $request->timing],
            ])->with('weather_forecast_spatial')
            ->whereHas('weather_forecast_spatial', function ($query) use ($spatial_id) {
                return $query->where('spatial_id', "=", $spatial_id);
            })
            ->orderBy('met_elements', 'asc')
            ->orderBy('from_datetime', 'desc')
            ->orderBy('weather_forecast_id', 'desc')->get();  
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
                        $tmp_arr['validtime'] = substr(preg_replace('/[\s\-:]/', '', $tmp_datetime),0,11);
                        $tmp_arr['minute'] = $selectResult->minute;
                        $values = json_decode($selectResult->contents);
                        $return_arr = ApiFunction::edit_contens_on_spatial($spatial_id,$values,
                                      $selectResult->lat_start,$selectResult->lon_start,$selectResult->lat_end,$selectResult->lon_end);
                        $tmp_arr['contents'] = $return_arr;
                        $tmp_arr['latInterval'] = $selectResult->lat_interval;
                        $tmp_arr['lonInterval'] = $selectResult->lon_interval;
                        $tmp_arr['latStart'] = $selectResult->lat_start;
                        $tmp_arr['lonStart'] = $selectResult->lon_start;
                        $tmp_arr['latEnd'] = $selectResult->lat_end;
                        $tmp_arr['lonEnd'] = $selectResult->lon_end;
                        $tmp_arr['grid'] = $selectResult->mesh;
                        $forecast[$selectResult->met_elements] = $tmp_arr;
                    }
                } 
            }
            //レスポンス用のjson配列編集
            $work [ 'other' ] = $forecast;
            $objects[] = $work;
            $objects_res = ['objects' => $objects];                              
            return  $objects_res;          
        }
    
            /*
            if ( $selectResults != false){
                $wk_mesh5 = "";
                foreach ( $selectResults as  $selectResult) {
                    if ($wk_mesh5 != $selectResult->mesh5) {
                        $work = ['spatial_id' => $spatial_id];
                        $timing = ['start_datetime' =>$selectResult->from_datetime]; 
                        $timing['end_datetime'] = $selectResult->to_datetime; 
                        $work['timing'] = $timing; 
                        $tmp_arr = ['mesh5' => $selectResult->mesh5];    
                        $tmp_arr['weather_time']  = $selectResult->from_datetime;
                        $tmp_arr['nowcast_flag'] = $selectResult->nowcast_flag;
                        $tmp_arr['forecast'] = $selectResult->forecast;
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
                    
    }*/

}
