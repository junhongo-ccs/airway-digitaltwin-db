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
use App\Http\Controllers\Api\RadioWaveVoxelController;
use App\Http\Controllers\Api\GroundFeatureVoxelController;
use App\Http\Controllers\Api\PopulationController;
use App\Http\Controllers\Api\WindController;
use App\Http\Controllers\Api\WeatherNowController;
use App\Http\Controllers\Api\WeatherForecastController;

class GeneralPurposeController extends Controller
{   
    public function get_general_purpose(Request $request) {
        try {
                //リクエストjsonのチェック
            if(json_last_error() !== JSON_ERROR_NONE) {
                throw new \Exception('Parameter error! Json Syntax Err!');
            }
            if(!$request->has('identification') ) {
                throw new \Exception('Parameter error! no [identification]');
            } else {
                $return_cd = ApiFunction::check_spatialId($request->identification);
                if( $return_cd != 17 ) {
                    throw new \Exception("Parameter error! " . identification . " is wrong format" );
                }
            }
            if(!$request->has('timing') ) {
                throw new \Exception('Parameter error! no [timing]');
            }  else {
                if (!ApiFunction::check_datetime($request->timing)) {
                    throw new \Exception('Parameter error! timing\'s format is wrong ');               
                }
            }
            if(!$request->has('requestType') ) {
                throw new \Exception('Parameter error! no [requestType]');
            }  
        
            switch ($request->requestType) {
                case "groundFeature":
                    $response_arr = GroundFeatureVoxelController::get_ground_feature_voxel($request);    
                    break;
                case "wind":
                    $response_arr = WindController::get_wind($request);
                    break;  
                case "weatherForecast":
                    $response_arr = WeatherForecastController::get_weather_forecast($request);
                    break;
                case "weatherNow":
                    $response_arr = WeatherNowController::get_weather_now($request);
                    break;
                case "radioWave":
                    $response_arr = RadioWaveVoxelController::get_radio_wave_voxel($request);
                    break;  
                case "averagePopulation":
                    $response_arr = PopulationController::get_average_population($request);    
                    break;
                case "area":
                    $response_arr = AreaObjectController::get_area($request);    
                    break;
                case "flightProhibitedArea":
                    $response_arr = FlightProhibitedAreaController::get_flight_prohibited_area($request);    
                    break;                 
                default:
                    throw new \Exception('Parameter error! wrong requestType]');
                    break;
        /*spatial_idsが配列でない時は配列に変換
        if (is_array($request["spatial_ids"])) {
            $tmp_request = $request["spatial_ids"];
        } else {
            $tmp_request = array($request["spatial_ids"]);
        }*/
        //近所の空間IDを集約した配列を作成
            }
            return response()->json(
                ['result' => $response_arr ], 200 ,['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES
             );
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
    }
}
