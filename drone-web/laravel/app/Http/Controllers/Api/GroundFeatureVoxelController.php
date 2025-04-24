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
use App\Models\GroundFeatureObject;

//use Exception;

class GroundFeatureVoxelController extends Controller
{   
    public static function get_ground_feature_voxel($request)
    {
        //タイプコードチェック
        if(!$request->has('other') ) {
            throw new \Exception('Parameter error! no [other]');
        } else {
            if(!isset($request['other']['typeCd'])) {
                throw new \Exception('Parameter error! no [typeCd].');
            }
        }
        $type_cd = $request->other["typeCd"];
        if ($type_cd == null ) {
            throw new \Exception('Parameter error!  no [typeCd].'); 
        }   
        if (!is_numeric($type_cd)) {
            throw new \Exception('Parameter error! [typeCd] is not numeric.'); 
        } 
                                   
        //空間IDチェック2
        $spatial_id = $request["identification"];
        //明細検索
        $wk_timing = $request->timing;
        $selectRes = GroundFeatureObject::where([
            ['spatial_id', '=', $spatial_id],
            ['from_datetime', '<=', $wk_timing],           
            ['object_cd', '=', $type_cd]
            ])->where(function($query) use ($wk_timing) {
                $query->where('to_datetime',   '>=', $wk_timing)
                      ->orWhere('to_datetime',   '=', null);
            })->orderBy('from_datetime', 'desc')
            ->orderBy('ground_feature_object_id', 'desc')
                ->first();
        //jsonファイル用配列(objects部)作成
        $objects = [];
        if(!$selectRes == null) {
            $work = ['spatialId' => $spatial_id];
            $timing = ['fromDatetime' =>$selectRes->from_datetime]; 
            $timing['endDatetime'] = $selectRes->to_datetime; 
            $work['timing'] = $timing; 
            $other = ['voxelBitFileName' => config('userapi.addHttpRootPath'). $selectRes->voxel_bit_file_path];
            $other['voxelBitSpatialZoomLevel'] = $selectRes->voxel_bit_spatial_zoom_level;
            $other['voxelBitEpsg'] = $selectRes->point_cloud_epsg;
            $work['other'] = $other; 
            $objects[] = $work;
        }
        $objects_res = ['objects' => $objects];                              
        return  $objects_res;             
    }

}