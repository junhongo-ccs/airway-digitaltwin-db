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
use App\Models\RadioWaveObject;

class RadioWaveVoxelController extends Controller
{   
    public static function get_radio_wave_voxel($request)
    {
        //項目チェック
        //Jsonチェック
            
        $spatial_id = $request["identification"];
        //明細検索
        $wk_timing = $request->timing;
        $selectRes = RadioWaveObject::where([
                ['spatial_id', '=', $spatial_id],
                ['from_datetime', '<=', $wk_timing]
                ])->where(function($query) use ($wk_timing) {
                    $query->where('to_datetime',   '>=', $wk_timing)
                          ->orWhere('to_datetime',   '=', null);
                })->orderBy('from_datetime', 'desc')
                ->orderBy('radio_wave_object_id', 'desc')
                ->first();
        //jsonファイル用配列(laz_files部)作成
        //$response_arr = [];
        $objects = [];
        if(!$selectRes == null) {
            $work = ['spatialId' => $spatial_id];
            $timing = ['fromDatetime' => $selectRes->from_datetime]; 
            $timing['endDatetime'] = $selectRes->to_datetime; 
            $work['timing'] = $timing; 
            $other = ['voxelBitFileName' => config('userapi.addHttpRootPath').$selectRes->voxel_bit_file_path];
            $other['voxelBitSpatialZoomLevel'] = $selectRes->voxel_bit_spatial_zoom_level;
            $other['voxelBitEpsg'] = $selectRes->point_cloud_epsg;
            $work['other'] = $other; 
            $objects[] = $work;
        }

        $objects_res = ['objects' => $objects];                              
        return  $objects_res;   
            
    }

}
