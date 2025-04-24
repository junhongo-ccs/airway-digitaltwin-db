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
use App\Models\DetailObjectMaster;
use App\Models\SpacialDetailObject;
use App\Models\SpacialDetailAttributeFile;
use ZipStream;
//use Exception;

class DetailSpatialVoxelController extends Controller
{   

    public function detail_object_voxel_get(Request $request)
    {
        $tmpDir = "";
        try {
            //項目チェック
            //Jsonチェック
            if(json_last_error() !== JSON_ERROR_NONE) {
                throw new \Exception('Json syntax error!');
            }
            if(!is_numeric($request["type_cd"])) {
                throw new \Exception('Parameter error! type_cd is not numeric');
            }  

            $type_cd = (int)$request["type_cd"];

            if($request["spatial_ids"] == null) {
                throw new \Exception('Parameter error! no spatial_ids');
            }  

            if (is_array($request["spatial_ids"])) {
                $spatial_ids = $request["spatial_ids"];
            } else {
                $spatial_ids = array($request["spatial_ids"]);
            }
            //Zip圧縮しながらstreamでのdowmload開始
            $zips = new ZipStream\ZipStream(
                outputName: '',
                // enable output of HTTP headers
                sendHttpHeaders: true ,
                contentDisposition:'attachment; filename=detail_object.zip' ,
                contentType: 'application/zip'
            ); 
            //個別地物空間テーブル検索   
            $selectResults = SpacialDetailObject::whereIn(
                'spatial_id', $spatial_ids
            )->with('detail_master')
            ->whereHas('detail_master', function ($query) use ($type_cd) {
                return $query->where('object_cd', "=", $type_cd);
            })
            ->orderBy('detail_object_id', 'asc')->get(); 
            if($selectResults == null) {
                throw new \Exception('Parameter error! Nothing was selected.');
            } 
            //存在するdetail_object_idを配列にする
            $selectIds = [];
            $OLD_KEY = "";
            foreach ($selectResults as $selectRes) {
                if ($selectRes->detail_object_id != $OLD_KEY) {
                    $selectIds[] = $selectRes->detail_object_id;
                    $OLD_KEY = $selectRes->detail_object_id;
                }
            }

            //検索結果のdetail_object_id分ループ開始
            $response_outs = []; 
            foreach ($selectIds as $selectId) {
                $response_out = [];
                //jsonファイル用配列(master部)作成
                $response_out =  ['detail_object_id' => $selectId ]; 
                $response_out['type_cd'] = $type_cd; 
                //voxelファイル処理
                $laz_files = [];
                foreach ($selectResults as $selectRes) {
                    if ($selectRes->detail_object_id == $selectId ) {
                        //zipにbitファイル追加
                        $tmpFileName = str_replace( '/' , '_' , $selectRes->spatial_id) . "_" . $type_cd . "_" . $selectId . ".laz" ;
                        $tmpFullPath = str_replace(config('userapi.driveFrom') , config('userapi.driveTo') , $selectRes->voxel_bit_file_path);
                        if (File::exists($tmpFullPath)) {
                            $zips->addFileFromPath(
                                fileName: $tmpFileName,
                                path: $tmpFullPath,
                            );
                        }
                        //jsonファイル用配列(master部)作成2
                        $response_out['detail_object_name'] = $selectRes->detail_master->detail_object_name;
                        $response_out['info'] = $selectRes->detail_master->info;
                        //jsonファイル用配列(laz_files部)作成
                        $voxel_bit = ['spatial_id' => $selectRes->spatial_id]; 
                        $voxel_bit['voxel_bit_file_name'] = $tmpFileName;
                        $voxel_bit['voxel_bit_spatial_zoom_level'] = $selectRes->voxel_bit_spatial_zoom_level;
                        $voxel_bit['voxel_bit_epsg'] = $selectRes->point_cloud_epsg;
                        $laz_files[] = $voxel_bit;                        
                    }
                }
                $response_out['laz_files'] = $laz_files;          
                //attributeファイル処理
                $selectAttResults = SpacialDetailAttributeFile::where([
                    ['detail_object_id', '=', $selectId ]
                ])->get();
                $attr_files = []; 
                foreach ($selectAttResults as $selectAttRes) {
                    $tmpFullPath = str_replace(config('userapi.driveFrom') , config('userapi.driveTo') , $selectAttRes->attribute_file_path);
                    if (File::exists($tmpFullPath)) {
                        $zips->addFileFromPath(
                            fileName: $selectId . "/" . $selectAttRes->attribute_file_name,
                            path: $tmpFullPath,
                        );
                    }
                    //jsonファイル用配列(attribute部)作成
                    $attr = ['detail_object_attribute_id' => $selectAttRes->id]; 
                    $attr['attribute_file_name'] = $selectAttRes->attribute_file_name;
                    $attr['attribute_info'] = $selectAttRes->attribute_info;
                    $attr_files[] = $attr; 
                } 
                $response_out['detail_object_attribute'] = $attr_files;    
                $response_outs[] = $response_out;       
            }
            $response_json = json_encode(['voxel_info' => $response_outs], JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
            $zips->addFile(fileName: 'info.json' , data: $response_json);       
                //検索結果からマスターの地物タイプコードで抽出
                /*$selectResults = [];
                foreach ($selectResultsTmps as $selectResultsTmp) {
                    if ($selectResultsTmp->detail_master->object_cd == $type_cd) {
                        $selectResults[] = $selectResultsTmp;
                    }
                }*/
            //zipsteamの終了
            $zips->finish();
        } catch (\Exception $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 500,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }                     
    }

}
