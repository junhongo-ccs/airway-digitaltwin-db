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
use App\Models\DroneRoute;
use App\Models\DroneRouteSpatial;
use App\Models\SpaceObject;
use ZipStream;
//use Exception;

class DroneRouteController extends Controller
{   
    public function drone_route(Request $request)
    {  
        //項目チェック
        try {
            if(json_last_error() !== JSON_ERROR_NONE) {
                throw new \Exception('Parameter error! Json Syntax Err!');
            }
            if(!$request->has('drone_route_id') ) {
                throw new \Exception('Parameter error! no [drone_route_id]');
            } else {
                if (!is_numeric($request->drone_route_id)) {
                    throw new \Exception('Parameter error! [drone_route_id]  is not numeric.');
                }
            }
            if(!$request->has('drone_route_name') ) {
                throw new \Exception('Parameter error! no [drone_route_name]');
            } else {
                if ($request->drone_route_name == null) {
                    throw new \Exception('Parameter error! no [drone_route_name]');               
                }
            }

            if(!$request->has('coordinates') ) {
                throw new \Exception('Parameter error! no [coordinates]');
            } else {
                if ($request->coordinates == null) {
                    throw new \Exception('Parameter error! no [coordinates]');               
                }
            }
            if(!$request->has('from_datetime') ) {
                throw new \Exception('Parameter error! no [from_datetime]');
            } else {
                if (!ApiFunction::check_datetime($request->from_datetime)) {
                    throw new \Exception('Parameter error! from_date\'s format is wrong ');               
                }
            }
            $wk_to_datetime = "9999-12-31 23:59:59";
            if(ApiFunction::check_datetime($request->to_datetime)) {
                $wk_to_datetime = $request->to_datetime;
            }
            
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }

        //トランザクション処理開始
        DB::beginTransaction();
        $response_json1 = json_encode($request->coordinates, JSON_UNESCAPED_UNICODE);
        try {
            $return_create = DroneRoute::Create(  
                ["drone_route_id"     => $request->drone_route_id,
                 "drone_route_name"   => $request->drone_route_name,
                // "radius"             => $request->radius,
                // "point_epsg"         => $crs,
                 "drone_polyline"     => $response_json1,
                 "from_datetime"      => $request->from_datetime,
                 "to_datetime"        => $wk_to_datetime,
                 "status"             => 1 ]
                );  
            DB::commit();

            //return response()->json(['drone_route_id' => $return_create->drone_route_id]);
        } catch (\Throwable $e) {
            DB::rollback();
            return response()->json([  'message' => 'db update error '.$e->getMessage()], 500,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }

    }

    public function get_drone_route(Request $request)
    
    {
        try {
            //項目チェック
            //Jsonチェック
            if(json_last_error() !== JSON_ERROR_NONE) {
                throw new \Exception('Json syntax error!');
            }

            if(!$request->has('drone_route_id') ) {
                throw new \Exception('Parameter error! no [drone_route_id]');
            } else {
                if (!is_numeric($request->drone_route_id)) {
                    throw new \Exception('Parameter error! [drone_route_id] is not numeric.');               
                }
            }

            //ドローン空路名検索
            $selectName = DroneRoute::find(
                $request->drone_route_id
            );
            if ($selectName == false){
                throw new \Exception('Parameter error! [drone_route_id] is not exist.');
            }               
            //DroneRouteSpatialからdrone_route_id検索
            $selectSpatials = DroneRouteSpatial::where(
                'drone_route_id', '=',$request->drone_route_id
            //)->orderBy('user_object_id', 'asc')->get();
            )->get();
            $jsonArr2 = [];
            if ($selectSpatials != false){
                //SpaceObjectから空間ID検索してjson明細編集
                foreach ($selectSpatials as $selectSpatial) {
                    $selectObject = SpaceObject::where([
                        ['spatial_id', '=',$selectSpatial->spatial_id],
                        ['object_cd', '=',200]
                    ])->orderBy('created_at', 'desc')->first();
                    if ($selectObject == false) {
                        continue;
                    }
                    $jsonArr1 = ['spatial_id' => $selectObject->spatial_id];    
                    $jsonArr1['voxel_bit_file_path'] = $selectObject->voxel_bit_file_path;
                    $jsonArr1['voxel_bit_spatial_zoom_level'] = $selectObject->voxel_bit_spatial_zoom_level;
                    $jsonArr1['voxel_bit_epsg'] = $selectObject->point_cloud_epsg;
                    $jsonArr2[] = $jsonArr1;
                } 
            }
            //レスポンス用のjson配列編集
            $jsonArr3 = [ 'drone_route_id' => $request->drone_route_id ];
            $jsonArr3 [ 'drone_route_name' ] = $selectName->drone_route_name;
            $jsonArr3 [ 'laz_files' ] = $jsonArr2;

            return response()->json(
                [ $jsonArr3 ], 200 ,['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
             );
           
        } catch (\Exception $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }   catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 500,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }                    
    }

}
