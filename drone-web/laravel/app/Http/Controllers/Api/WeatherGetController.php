<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Http\Controllers\Api\ApiFunction;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Log;
use Illuminate\Support\Facades\Storage;
use Illuminate\Support\Facades\File;
use Illuminate\Support\Facades\Auth;
use Illuminate\Support\Facades\Http;

class WeatherGetController extends Controller
{   
    public function weather_current_get(Request $request)
    {
    try {
            //リクエストjsonのチェック
        if(json_last_error() !== JSON_ERROR_NONE) {
            return response()->json([  'message' => 'Json syntax error!'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
        if($request["spatial_ids"] == null) {
            return response()->json([  'message' => 'Parameter error! no spatial_ids'], 400,
        ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        }    
        //spatial_idsが配列でない時は配列に変換
        if (is_array($request["spatial_ids"])) {
            $tmp_request = $request["spatial_ids"];
        } else {
            $tmp_request = array($request["spatial_ids"]);
        }
        //近所の空間IDを集約した配列を作成
        $base_array = WeatherGetController::base_array_get($tmp_request);
        
    } catch (\Throwable $e) {
        return response()->json([  'message' => $e->getMessage()], 400,
        ['Content-Type' => 'application/json;charset=UTF-8'],
        JSON_UNESCAPED_UNICODE
        );
    }
        
        $address = config('userapi.weatherAddressCurrent') ;
        $auth = config('userapi.weatherAuthKey') ;
        //$pointX = 35;
        //$pointY = 139;
        //外部APIからデータをgetする
        $response_put = [];
        $tmp_out1 = [];
        //$resp = Http::get($address."/".$year."/".$month."/".$day."/".$hour."/".$level."/".$x."/".$y."/?auth=".$auth);
        //$resp = Http::get("https://api.weatherapi.com/v1/current.json?q=35.55194880177575,139.27602407291198&lang=ja&key=29b28e474b28454baf785103240909");
        foreach($base_array as $key => $spatial_arr) {
            $point_arr2 = explode('@', $key);
            $pointX2    = $point_arr2[0];
            $pointY2    = $point_arr2[1];
            $resp = Http::get($address."?q=".$pointX2.",".$pointY2."&lang=ja&key=".$auth);
            //エラー時はエラー情報を返す
            if($resp->failed()) {
                return response()->json([  'message' => "external API request error!! " . $resp  ], $resp->status(),
                    ['Content-Type' => 'application/json;charset=UTF-8'],
                    JSON_UNESCAPED_UNICODE);
            }
            $tmp_out2 = ["spatial_ids" => $spatial_arr];
            $tmp_out3 = json_decode($resp, true);
            $tmp_out4 = array_merge($tmp_out2, $tmp_out3);
            $tmp_out1[] = $tmp_out4;
            
        }
        $response_out = ["weather_info" => $tmp_out1];
        return response()->json(
            $response_out,200,['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
        );
    }

    public function weather_forecast_get(Request $request)
    {
        try {
            //リクエストjsonのチェック
            if(json_last_error() !== JSON_ERROR_NONE) {
                return response()->json([  'message' => 'Json syntax error!'], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
            }
            if($request["spatial_ids"] == null) {
                return response()->json([  'message' => 'Parameter error! no spatial_ids'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
            }    
            //spatial_idsが配列でない時は配列に変換
            if (is_array($request["spatial_ids"])) {
                $tmp_request = $request["spatial_ids"];
            } else {
                $tmp_request = array($request["spatial_ids"]);
            }
            if(!is_numeric($request["yyyy"])) {
                throw new \Exception('Parameter error! year not numeric');
            }     
            if(!is_numeric($request["mm"])) {
                throw new \Exception('Parameter error! month not numeric');
            }  
            if(!is_numeric($request["dd"])) {
                throw new \Exception('Parameter error! day not numeric');
            }
            //近所の空間IDを集約した配列を作成
            $base_array = WeatherGetController::base_array_get($tmp_request);

        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
        
        $address = config('userapi.weatherAddressForecast') ;
        $auth = config('userapi.weatherAuthKey') ;
   
        //外部APIからデータをgetする
        $response_put = [];
        $tmp_out1 = [];
        $request_date = $request["yyyy"]."-".$request["mm"]."-".$request["dd"];
        foreach($base_array as $key => $spatial_arr) {
            $point_arr2 = explode('@', $key);
            $pointX2    = $point_arr2[0];
            $pointY2    = $point_arr2[1];
            $resp = Http::get($address."?q=".$pointX2.",".$pointY2."&dt=".$request_date."&lang=ja&key=".$auth);
            //エラー時はエラー情報を返す
            if($resp->failed()) {
                return response()->json([  'message' => "external API request error!! " . $resp  ], $resp->status(),
                    ['Content-Type' => 'application/json;charset=UTF-8'],
                    JSON_UNESCAPED_UNICODE);
            }
            $tmp_out2 = ["spatial_ids" => $spatial_arr];
            $tmp_out3 = json_decode($resp, true);
            $tmp_out4 = array_merge($tmp_out2, $tmp_out3);
            $tmp_out1[] = $tmp_out4;
        }
        $response_out = ["weather_info" => $tmp_out1];
        return response()->json(
            $response_out,200,['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
        );
        
    }

    public function weather_history_get(Request $request)
    {
        try {
            //リクエストjsonのチェック
            if(json_last_error() !== JSON_ERROR_NONE) {
                return response()->json([  'message' => 'Json syntax error!'], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
            }
            if($request["spatial_ids"] == null) {
                return response()->json([  'message' => 'Parameter error! no spatial_ids'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
            }    
            //spatial_idsが配列でない時は配列に変換
            if (is_array($request["spatial_ids"])) {
                $tmp_request = $request["spatial_ids"];
            } else {
                $tmp_request = array($request["spatial_ids"]);
            }
            if(!is_numeric($request["yyyy"])) {
                throw new \Exception('Parameter error! year not numeric');
            }     
            if(!is_numeric($request["mm"])) {
                throw new \Exception('Parameter error! month not numeric');
            }  
            if(!is_numeric($request["dd"])) {
                throw new \Exception('Parameter error! day not numeric');
            }
            //近所の空間IDを集約した配列を作成
            $base_array = WeatherGetController::base_array_get($tmp_request);

        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
        
        $address = config('userapi.weatherAddressHistory') ;
        $auth = config('userapi.weatherAuthKey') ;
   
        //外部APIからデータをgetする
        $response_put = [];
        $tmp_out1 = [];
        $request_date = $request["yyyy"]."-".$request["mm"]."-".$request["dd"];
        foreach($base_array as $key => $spatial_arr) {
            $point_arr2 = explode('@', $key);
            $pointX2    = $point_arr2[0];
            $pointY2    = $point_arr2[1];
            $resp = Http::get($address."?q=".$pointX2.",".$pointY2."&dt=".$request_date."&lang=ja&key=".$auth);
            //エラー時はエラー情報を返す
            if($resp->failed()) {
                return response()->json([  'message' => "external API request error!! " . $resp  ], $resp->status(),
                    ['Content-Type' => 'application/json;charset=UTF-8'],
                    JSON_UNESCAPED_UNICODE);
            }
            $tmp_out2 = ["spatial_ids" => $spatial_arr];
            $tmp_out3 = json_decode($resp, true);
            $tmp_out4 = array_merge($tmp_out2, $tmp_out3);
            $tmp_out1[] = $tmp_out4;
        }
        $response_out = ["weather_info" => $tmp_out1];
        return response()->json(
            $response_out,200,['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
        );
        
    }

    //空間IDの配列から中心座標の小数点２桁で集約した配列を作成する
    public function base_array_get($spatial_ids)
    {
        $crs = '4326'; 
        $base_array = [];
        foreach($spatial_ids as $val) { 
            //高さは何が入力されても0に置換する          
            $tmp_spatial_id = explode('/', $val);
            $spatial_id = $tmp_spatial_id[0]."/0/".$tmp_spatial_id[2]."/".$tmp_spatial_id[3];
            //空間IDのフォーマットチェック 
            $return_cd = ApiFunction::check_spatialId($spatial_id);
            if( $return_cd != 17 ) {
                throw new \Exception('Parameter error! wrong spatial_id');
            }
         
            $points = shell_exec( config('userapi.exepath') . ' 4 '  . $spatial_id . ' ' . $crs);
            //エラーで戻ってきた時の処理
            if($points == null || $points == false) {
                throw new \Exception('Parameter or server error!');
            }
            if($points == 'wrong parameters!') {
                throw new \Exception('Parameter server error!');
            }
            //Log::info($spatial_id);
            //Log::info($points);
            //$poins(空白1文字で区切られている中心座標)を配列に追加 
            $point_arr = explode(' ', $points);
            $pointX = number_format($point_arr[0], 2);
            $pointY = number_format($point_arr[1], 2);
            $array_key = $pointX."@".$pointY; 
            $base_array[$array_key][] = $val;
        }
        return $base_array;

    }
   
}
