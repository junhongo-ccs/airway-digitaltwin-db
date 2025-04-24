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
use App\Models\UserObjectMaster;
use App\Models\SpacialUserObject;
use App\Models\SpacialUserAttributeFile;
use ZipStream;
//use Exception;

class UserSpatialVoxelController extends Controller
{   
    public function user_object_voxel_set(Request $request)
    {  
        $file_name_tmp = "";
        //項目チェック
        try {
            if(!$request->has('user_object_name') ) {
                throw new \Exception('Parameter error! no [user_object_name]');
            } else {
                if ($request->user_object_name == null) {
                    throw new \Exception('Parameter error! no [user_object_name]');
                }
            }
            if(!$request->has('voxel_bit_files') ) {
                throw new \Exception('Parameter error! no [voxel_bit_files]');
            } else {
                if ($request->voxel_bit_files == null) {
                    throw new \Exception('Parameter error! no [voxel_bit_files]');               
                }
            }
            if(!$request->has('voxel_bit_spatial_zoom_level') ) {
                throw new \Exception('Parameter error! no [voxel_bit_spatial_zoom_level]');
            } else {
                if (!is_numeric($request->voxel_bit_spatial_zoom_level)) {
                    throw new \Exception('Parameter error! [voxel_bit_spatial_zoom_level] is not numeric.');               
                }
            }
            if(!$request->has('voxel_bit_epsg') ) {
                throw new \Exception('Parameter error! no [voxel_bit_epsg]');
            } else {
                if (!is_numeric($request->voxel_bit_epsg)) {
                    throw new \Exception('Parameter error! [voxel_bit_epsg] is not numeric.' );                
                }
            }
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }
        //user_object_infoの設定がない場合は空文字
        if(!$request->has('user_object_info') ) {
            $user_object_info = "";
        } else {
            if ($request->user_object_info == null) {
                $user_object_info = "";
            } else {
                $user_object_info = $request->user_object_info;
            } 
        }
        //voxel_bit_filesが配列でない場合、配列に変更
        if (is_array($request->file("voxel_bit_files"))) {
            $voxel_bit_files = $request->file("voxel_bit_files");
        } else {
            $voxel_bit_files = array($request->file("voxel_bit_files"));
        }
        //トランザクション処理開始
        DB::beginTransaction();
        try {
            $return_create = UserObjectMaster::create([  
                "group_id"           => Auth::user()->group_id,
                "user_object_name"   => $request->user_object_name,
                "info"               => $user_object_info
                ]);  
        
            foreach($voxel_bit_files as $voxel_bit_file) {

                $file_name_tmp = $voxel_bit_file->store('public');
                $file_name_tmp = storage_path('app/') . $file_name_tmp;

                //ファイル保管場所フォルダを作成
                $user_object_id = $return_create->user_object_id;
                $file_name_original = $voxel_bit_file->getClientOriginalName();
                $file_path = pathinfo($file_name_original);
                $spatial_id = str_replace( '_' , '-' , $file_path['filename']);
                $spatial_id_db = str_replace( '_' , '/' , $file_path['filename']);
                $return_cd = ApiFunction::check_spatialId($spatial_id_db);
                if( $return_cd != 17 ) {
                    if (File::exists($file_name_tmp)) {
                        File::delete($file_name_tmp);
                    }
                    return response()->json([ 'message' => "Parameter error! " . $spatial_id_db . " is wrong spatial_id"], 400,
                    ['Content-Type' => 'application/json;charset=UTF-8'],
                    JSON_UNESCAPED_UNICODE
                        );
                }
            
                $copyTo = config('userapi.userpath') . '/' . Auth::user()->group_id . '/' . $user_object_id . '/voxel/' . $spatial_id. '/';
                if (!is_dir($copyTo)) {
                    mkdir($copyTo, 0777, true); 
                }
                //アップロードファイルを格納先フォルダにcopy&table spacial_user_objectに挿入
                
                File::copy( $file_name_tmp, $copyTo.$file_name_original);

                SpacialUserObject::create([  
                        "user_object_id"       => $user_object_id,
                        "spatial_id"           => $spatial_id_db,
                        "voxel_bit_file_name"  => $file_name_original,
                        "voxel_bit_file_path"  => $copyTo.$file_name_original,
                        "voxel_bit_spatial_zoom_level" => $request->voxel_bit_spatial_zoom_level,
                        "point_cloud_epsg" => $request->voxel_bit_epsg,
                        "update_memo"          => date('Y-m-d H:i:s')
                ]);  
                
                if (File::exists($file_name_tmp)) {
                    File::delete($file_name_tmp);
                }
            }    
            DB::commit();
            return response()->json(['user_object_id' => $return_create->user_object_id]);
        } catch (\Throwable $e) {
            DB::rollback();
            if (File::exists($file_name_tmp)) {
                File::delete($file_name_tmp);
            }
            return response()->json([  'message' => 'update error '.$e->getMessage()], 500,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
     
    }

    public function user_object_voxel_get_id(Request $request, $user_object_id)
    {
        $tmpDir = "";
        try {
            //項目チェック
            //$user_object_idのチェック   
            if(!is_numeric($user_object_id)) {
                throw new \Exception('Parameter error! user_object_id not numeric');
            } 
            //グループIDチェック
            $group_id_now = Auth::user()->group_id;
            $masterResult = UserObjectMaster::where([
                ['user_object_id', '=', $user_object_id]
            ])->first();

            if($masterResult == null) {
                throw new \Exception('Parameter error! wrong user_object_id ');
            } 

            if($group_id_now != $masterResult->group_id) {
                throw new \Exception('Parameter error! wrong group_id ');
            } 
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }       

        try {
            //Zip圧縮しながらstreamでのdowmload開始
            $zips = new ZipStream\ZipStream(
                outputName: '',
                // enable output of HTTP headers
                sendHttpHeaders: true ,
                contentDisposition:'attachment; filename=user_object_id_' . $user_object_id . '.zip' ,
                contentType: 'application/zip'
            ); 
            
            $selectResults = SpacialUserObject::where([
                ['user_object_id', '=', $user_object_id]
            ])->get();
            //lazファイルをzipに加える　lazファイル情報をjsonに加える
            $laz_files = []; 
            foreach ($selectResults as $selectRes) {
                $fileNameFull = str_replace(config('userapi.driveFrom') , config('userapi.driveTo') , $selectRes->voxel_bit_file_path);
                if (File::exists($fileNameFull)) {
                    $zips->addFileFromPath(
                        fileName: $selectRes->voxel_bit_file_name,
                        path: $fileNameFull,
                    );
                }
                $voxel_bit = ['spatial_id' => $selectRes->spatial_id]; 
                $voxel_bit['voxel_bit_file_name'] = $selectRes->voxel_bit_file_name;
                $voxel_bit['voxel_bit_spatial_zoom_level'] = $selectRes->voxel_bit_spatial_zoom_level;
                $voxel_bit['voxel_bit_epsg'] = $selectRes->point_cloud_epsg;
                $laz_files[] = $voxel_bit;     
            } 

            $selectAttResults = SpacialUserAttributeFile::where([
                ['user_object_id', '=', $user_object_id]
            ])->get();
            $attr_files = []; 
            foreach ($selectAttResults as $selectAttRes) {
                //$tmpfilename = $user_object_id . "/" . $selectAttRes->voxel_attribute_file_name;
                $fileNameFull = str_replace(config('userapi.driveFrom') , config('userapi.driveTo') , $selectAttRes->attribute_file_path);
                if (File::exists($fileNameFull)) {
                    $zips->addFileFromPath(
                        fileName: $user_object_id . "/" . $selectAttRes->attribute_file_name,
                        path: $fileNameFull,
                    );
                }
                $attr = ['user_object_attribute_id' => $selectAttRes->id]; 
                $attr['attribute_file_name'] = $selectAttRes->attribute_file_name;
                $attr['attribute_info'] = $selectAttRes->attribute_info;
                $attr_files[] = $attr;     
            }   
            //json用の連想配列を作成  
            $response_out =  ['user_object_id' => $user_object_id]; 
            $response_out['user_object_name'] = $masterResult->user_object_name;
            $response_out['info'] = $masterResult->info;
            $response_out['laz_files'] = $laz_files;
            $response_out['user_object_attributes'] = $attr_files;

            
            $response_json = json_encode(['voxel_info' => array($response_out)],JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
            $zips->addFile(fileName: 'info.json' , data: $response_json);
            //zipsteamの終了
            $zips->finish();
           
        //例外発生時処理     
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 500,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }       
    }

    public function user_object_voxel_get(Request $request)
    {
        try {
            //項目チェック
            //Jsonチェック
            if(json_last_error() !== JSON_ERROR_NONE) {
                throw new \Exception('Json syntax error!');
            }
            if($request["spatial_ids"] == null) {
                throw new \Exception('Parameter error! no spatial_ids');
            }  

            if (is_array($request["spatial_ids"])) {
                $spatial_ids = $request["spatial_ids"];
            } else {
                $spatial_ids = array($request["spatial_ids"]);
            }
            //グループIDセット
            $group_id_now = Auth::user()->group_id;

            //Zip圧縮しながらstreamでのdowmload開始
            $zips = new ZipStream\ZipStream(
                outputName: '',
                // enable output of HTTP headers
                sendHttpHeaders: true ,
                contentDisposition:'attachment; filename=user_object.zip' ,
                contentType: 'application/zip'
            ); 
            //空間IDチェック
            foreach ($spatial_ids as $spatial_id) {
                $return_cd = ApiFunction::check_spatialId($spatial_id);
                if( $return_cd != 17 ) {
                    throw new \Exception("Parameter error! " . $spatial_id . " is wrong spatial_id" );
                }
            }
            //明細検索
            $selectResults = SpacialUserObject::whereIn(
                'spatial_id', $spatial_ids
            )->orderBy('user_object_id', 'asc')->get();
            //存在するuser_object_idを配列にする
            $selectIds = [];
            $OLD_KEY = "";
            foreach ($selectResults as $selectRes) {
                if ($selectRes->user_object_id != $OLD_KEY) {
                    $selectIds[] = $selectRes->user_object_id;
                    $OLD_KEY = $selectRes->user_object_id;
                }
            }
            //検索結果ループ開始
            $response_outs = []; 
            foreach ($selectIds as $selectId) {
                $response_out = [];
                //グループIDチェック
                $masterResult = UserObjectMaster::find(
                    $selectId
                );
                if($masterResult == null) {
                    throw new \Exception('Parameter error! wrong user_object_id ');
                } 
                if($group_id_now != $masterResult->group_id) {
                    throw new \Exception('Parameter error! wrong group_id ');
                }
                //jsonファイル用配列(master部)作成
                $response_out =  ['user_object_id' => $selectId ]; 
                $response_out['user_object_name'] = $masterResult->user_object_name;
                $response_out['info'] = $masterResult->info;
                //voxelファイル処理
                $laz_files = [];
                foreach ($selectResults as $selectRes) {
                    if ($selectRes->user_object_id == $selectId ) {
                        //zipにbitファイル追加
                        $tmpFileName = str_replace( '/' , '_' , $selectRes->spatial_id) . "_" . $selectId . ".laz" ;
                        $fileNameFull = str_replace(config('userapi.driveFrom') , config('userapi.driveTo') , $selectRes->voxel_bit_file_path);
                        if (File::exists($fileNameFull)) {
                            $zips->addFileFromPath(
                                fileName: $tmpFileName,
                                path: $fileNameFull,
                            );
                        }
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
                $selectAttResults = SpacialUserAttributeFile::where([
                    ['user_object_id', '=', $selectId ]
                ])->get();
                $attr_files = []; 
                foreach ($selectAttResults as $selectAttRes) {
                    $fileNameFull = str_replace(config('userapi.driveFrom') , config('userapi.driveTo') , $selectAttRes->attribute_file_path);
                    if (File::exists($fileNameFull)) {
                        $zips->addFileFromPath(
                            fileName: $selectId . "/" . $selectAttRes->attribute_file_name,
                            path: $fileNameFull,
                        );
                    }
                    //jsonファイル用配列(attribute部)作成
                    $attr = ['user_object_attribute_id' => $selectAttRes->id]; 
                    $attr['attribute_file_name'] = $selectAttRes->attribute_file_name;
                    $attr['attribute_info'] = $selectAttRes->attribute_info;
                    $attr_files[] = $attr; 
                } 
                $response_out['user_object_attribute'] = $attr_files;    
                $response_outs[] = $response_out;       
            }
            $response_json = json_encode(['voxel_info' => $response_outs], JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
            $zips->addFile(fileName: 'info.json' , data: $response_json);      
            //zipsteamの終了
            $zips->finish();
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

    public function user_object_voxel_updt(Request $request, $user_object_id)
    {   
        $file_name_tmp = "";
        //項目チェック
        try {
            //$user_object_idのチェック  
            if(!is_numeric($user_object_id)) {
                throw new \Exception('Parameter error! user_object_id not numeric');
            }    
            //グループIDチェック
            $group_id_now = Auth::user()->group_id;
            $masterResult = UserObjectMaster::find($user_object_id);
            if($masterResult == null) {
                throw new \Exception('Parameter error! wrong user_object_id');
            } 
            if($group_id_now != $masterResult->group_id) {
                throw new \Exception('wrong group_id');
            } 
            //user_object_nameチェック
            if(!$request->has('user_object_name') ) {
                throw new \Exception('Parameter error! no [user_object_name]');
            } else {
                if ($request->user_object_name == null) {
                    throw new \Exception('Parameter error! no [user_object_name]');
                }
            }
            //voxel_bit_filesチェック
            if(!$request->has('voxel_bit_files') ) {
                throw new \Exception('Parameter error! no [voxel_bit_files]');
            } else {
                if ($request->voxel_bit_files == null) {
                    throw new \Exception('Parameter error! no [voxel_bit_files]');
                }
            }
            //voxel_bit_spatial_zoom_levelチェック
            if(!$request->has('voxel_bit_spatial_zoom_level') ) {
                throw new \Exception('Parameter error! no [voxel_bit_spatial_zoom_level]');
            } else {
                if (!is_numeric($request->voxel_bit_spatial_zoom_level)) {
                    throw new \Exception('Parameter error! [voxel_bit_spatial_zoom_level] is not numeric.');
                
                }
            }
            if(!$request->has('voxel_bit_epsg') ) {
                throw new \Exception('Parameter error! no [voxel_bit_epsg]');
            } else {
                if (!is_numeric($request->voxel_bit_epsg)) {
                    throw new \Exception('Parameter error! [voxel_bit_epsg] is not numeric.' );                
                }
            }
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }
        //配列でない場合、配列に変更
        if (is_array($request->file("voxel_bit_files"))) {
            $voxel_bit_files = $request->file("voxel_bit_files");
        } else {
            $voxel_bit_files = array($request->file("voxel_bit_files"));
        }
        //user_object_infoの設定がない場合は1文字スペース
        if(!$request->has('user_object_info') ) {
            $user_object_info = $masterResult->user_object_info;
        } else {
            if ($request->user_object_info == null) {
                $user_object_info = $masterResult->user_object_info;
            } else {
                $user_object_info = $request->user_object_info;
            } 
        }
        DB::beginTransaction();
        try {
            //voxelファイルの削除
            $delDir = config('userapi.userpath') . '/' . $group_id_now . '/' . $user_object_id . '/voxel';
            if (is_dir($delDir)) {
                File::deleteDirectory($delDir);
            }
            //SpacialUserObjectの削除
            SpacialUserObject::where([
                ['user_object_id', '=', $user_object_id]
            ])->delete(); 
            //UserObjectMasterの更新
            $masterResult->update([  
                "user_object_name"   => $request->user_object_name,
                "info"               => $user_object_info
                ]);  
        
            foreach($voxel_bit_files as $voxel_bit_file) {

                $file_name_tmp = $voxel_bit_file->store('public');
                $file_name_tmp = storage_path('app/') . $file_name_tmp;
    
                //ファイル保管場所フォルダを作成
                $file_name_original = $voxel_bit_file->getClientOriginalName();
                $file_path = pathinfo($file_name_original);
                $spatial_id = str_replace( '_' , '-' , $file_path['filename']);
                $spatial_id_db = str_replace( '_' , '/' , $file_path['filename']);
                $return_cd = ApiFunction::check_spatialId($spatial_id_db);
                if( $return_cd != 17 ) {
                    if (File::exists($file_name_tmp)) {
                        File::delete($file_name_tmp);
                    }
                    return response()->json([ 'message' => "Parameter error! " . $spatial_id_db . " is wrong spatial_id"], 400,
                    ['Content-Type' => 'application/json;charset=UTF-8'],
                    JSON_UNESCAPED_UNICODE
                        );
                }
                
                $copyTo = config('userapi.userpath') . '/' . Auth::user()->group_id . '/' . $user_object_id . '/voxel/' . $spatial_id. '/';
                if (!is_dir($copyTo)) {
                    mkdir($copyTo, 0777, true); 
                }
                //アップロードファイルを格納先フォルダにcopy&table spacial_user_objectに挿入
                    
                File::copy( $file_name_tmp, $copyTo.$file_name_original);
    
                SpacialUserObject::create([  
                        "user_object_id"       => $user_object_id,
                        "spatial_id"           => $spatial_id_db,
                        "voxel_bit_file_name"  => $file_name_original,
                        "voxel_bit_file_path"  => $copyTo.$file_name_original,
                        "voxel_bit_spatial_zoom_level" => $request->voxel_bit_spatial_zoom_level,
                        "point_cloud_epsg"     => $request->voxel_bit_epsg,
                        "update_memo"          => date('Y-m-d H:i:s')
                ]);  
                    
                if (File::exists($file_name_tmp)) {
                    File::delete($file_name_tmp);
                }
            }    
            DB::commit();
        } catch (\Throwable $e) {
            if (File::exists($file_name_tmp)) {
                File::delete($file_name_tmp);
            }
            DB::rollback();
                return response()->json([  'message' => 'update error '.$e->getMessage()], 500,
            ['Content-Type' => 'application/json;charset=UTF-8'],
               JSON_UNESCAPED_UNICODE
                );
        }        
    }
    
    public function user_object_voxel_del(Request $request, $user_object_id)
    {      
        try{
            //$user_object_idのチェック  
            if(!is_numeric($user_object_id)) {
                throw new \Exception('Parameter error! user_object_id not numeric');
            } 

            //グループIDチェック
            $group_id_now = Auth::user()->group_id;
            $masterResult = UserObjectMaster::where([
                ['user_object_id', '=', $user_object_id]
            ])->first();
            if($masterResult == null) {
                throw new \Exception('Parameter error! wrong user_object_id ');
            } 
            if($group_id_now != $masterResult->group_id) {
                throw new \Exception('Parameter error! wrong group_id ');
            } 
        } catch (\Throwable $e) {
            return response()->json([  'message' => 'update error '.$e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
        }
        DB::beginTransaction();
        try {
            //各テーブルの削除
            $masterResult->delete();  
            SpacialUserObject::where([
                ['user_object_id', '=', $user_object_id]
            ])->delete(); 
            SpacialUserAttributeFile::where([
                ['user_object_id', '=', $user_object_id]
            ])->delete();
            DB::commit();
        } catch (\Throwable $e) {
            DB::rollback();
                return response()->json([  'message' => 'update error '.$e->getMessage()], 500,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
        }
    }
}
