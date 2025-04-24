<?php

use Illuminate\Http\Request;
use Illuminate\Support\Facades\File;
use Illuminate\Support\Facades\Route;
use App\Http\Controllers\Api\UserController;
use App\Http\Controllers\Api\UserApiController;
use App\Http\Controllers\Api\DroneRouteController;
use App\Http\Controllers\Api\PopulationController;
use App\Http\Controllers\Api\RadioWaveVoxelController;
use App\Http\Controllers\Api\GroundFeatureVoxelController;
use App\Http\Controllers\Api\AreaObjectController;
use App\Http\Controllers\Api\GeneralPurposeController;
use App\Http\Controllers\Api\WindController;
use App\Http\Controllers\Api\WeatherNowController;
use App\Http\Controllers\Api\WeatherForecastController;
use App\Http\Controllers\Api\FlightProhibitedAreaController;
//use App\Http\Controllers\Api\DetailSpatialVoxelController;
//use App\Http\Controllers\Api\DetailSpatialAttributeController;
//use App\Http\Controllers\Api\StayMoveController;
//use App\Http\Controllers\Api\WeatherGetController;

/*
|--------------------------------------------------------------------------
| API Routes
|--------------------------------------------------------------------------
|
| Here is where you can register API routes for your application. These
| routes are loaded by the RouteServiceProvider and all of them will
| be assigned to the "api" middleware group. Make something great!
|
*/

// Laravel Sanctumの使い方
// https://codelikes.com/use-laravel-sanctum/

//Api/UserControllerで認証＆Token発行

Route::middleware([
    'api'
])->group(function () {
    // http://localhost/api/login
    Route::match(['post'],'/login',  [UserController::class, 'login'])->name('login');
});

//Route::middleware(['auth:sanctum'])->group(function(){
    // http://localhost/api/spatial_voxel
    Route::match(['post'],'/spatial_voxel',  [UserApiController::class, 'spatial_voxel'])->name('spatial_voxel');
    // http://localhost/api/drone_route
    Route::match(['post'],'/drone_route',  [DroneRouteController::class, 'drone_route'])->name('drone_route');
    // http://localhost/api/drone_route
    Route::match(['get'],'/drone_route',  [DroneRouteController::class, 'get_drone_route'])->name('get_drone_route');
    // http://localhost/api/ground_featur_voxel
    Route::match(['get'],'/ground_feature_voxel',  [GroundFeatureVoxelController::class, 'get_ground_feature_voxel'])->name('get_ground_feature_voxel');
    // http://localhost/api/ground_featur_voxel
    Route::match(['get'],'/radio_wave_voxel',  [RadioWaveVoxelController::class, 'get_radio_wave_voxel'])->name('get_radio_wave_voxel');
    // http://localhost/api/average_population
    Route::match(['get'],'/average_population',  [PopulationController::class, 'get_average_population'])->name('get_average_population');
    // http://localhost/api/area
    Route::match(['post'],'/area',  [AreaObjectController::class, 'set_area_object_masters'])->name('set_area_object_masters');
    // http://localhost/api/general_purpose
    Route::match(['get'],'/general_purpose',  [GeneralPurposeController::class, 'get_general_purpose'])->name('get_general_purpose');
    // http://localhost/api/wind
    Route::match(['post'],'/wind',  [WindController::class, 'set_wind'])->name('set_wind');
    // http://localhost/api/weather/now
    Route::match(['post'],'/weather/now',  [WeatherNowController::class, 'set_weather_now'])->name('set_weather_now');
    // http://localhost/api/weather/forecast
    Route::match(['post'],'/weather/forecast',  [WeatherForecastController::class, 'set_weather_forecast'])->name('set_weather_forecast');
    // http://localhost/api/flight_prohibited_area
    Route::match(['post'],'/flight_prohibited_area',  [FlightProhibitedAreaController::class, 'set_flight_prohibited_area'])->name('set_flight_prohibited_area');
    
    // protected file
    Route::get('/protected/{path?}', function (Request $request,$path='') {
        if($path==='') abort(404);
        $rp = base_path('protected/'.$path);
        if(File::exists($rp)){
            return response()->file($rp);
        }else{
            abort(404);
        }
    })->where('path', '.*');
//});
