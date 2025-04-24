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
use App\Models\SpacialDetailAttributeFile;
use ZipStream;

class DetailSpatialAttributeController extends Controller
{
    
    public function detail_object_attribute_set(Request $request, $detail_object_id)
    {   
        $file_name_tmp = "";
        $file_flg = false;
        //項目チェック
        try {
            //attribute_infoが設定されていなければnullを代入
            if(!$request->has('attribute_info')) {
                $attribute_info = null;
            } else {
                $attribute_info = $request->attribute_info;
            }
            
            //$detail_object_idのチェック   
            if(!is_numeric($detail_object_id)) {
                throw new \Exception('Parameter error! detail_object_id not numeric');
            } 

            //ファイルが複数だとエラー     
            if($request->has('attribute_file')) {
                $attribute_file = $request->file("attribute_file");
                if (is_array($attribute_file)) {
                    throw new \Exception('Parameter error! too many attribute_files');
                }
                if (!$attribute_file == null) {
                    $file_flg = true;
                }
            }
            //detail_object_id存在チェック
            $masterResult = DetailObjectMaster::find(
                $detail_object_id
            );
            if ($masterResult == null) {
                throw new \Exception('Parameter error! detail_object_id not exists');
            }           
        //項目チェックエラー処理   
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }            
        //トランザクション処理開始
        DB::beginTransaction();
        try {
            if ( $file_flg ) {
                $copyTo = config('userapi.detailpath') . '/' . $detail_object_id . '/attribute/';
        
                if (!is_dir($copyTo)) {
                    mkdir($copyTo, 0777, true);
                }      
                //所定の場所にcopy
                $file_name_tmp = $attribute_file->store('public');
                $file_name_tmp = storage_path('app/') . $file_name_tmp;

                $file_name_original = $attribute_file->getClientOriginalName();
                File::copy( $file_name_tmp , $copyTo.$file_name_original);
                $selectRes = SpacialDetailAttributeFile::where([
                    ['detail_object_id', '=', $detail_object_id],
                    ['attribute_file_name', '=', $file_name_original]                   
                ])->count(); 
                if ($selectRes == 0)  {
                    $return_create = SpacialDetailAttributeFile::create([  
                        "detail_object_id"       => $detail_object_id,
                        "attribute_file_name"    => $file_name_original,
                        "attribute_file_path"  => $copyTo.$file_name_original,
                        "attribute_info"    => $attribute_info,
                        "update_memo"          => date('Y-m-d H:i:s')
                    ]);  
                } else {
                    throw new \Exception('Parameter error! File exists');
                }
          
                if (File::exists($file_name_tmp)) {
                    File::delete($file_name_tmp);
                }
            } else {
                $return_create = SpacialDetailAttributeFile::create([  
                    "detail_object_id"       => $detail_object_id,
                    "attribute_info"    => $attribute_info,
                    "update_memo"          => date('Y-m-d H:i:s')
                ]);  
            }
            DB::commit();               
            return response()->json(['detail_object_attribute_id' => $return_create->id]);
        //DBエラー処理
        } catch (\Exception $e) {
            DB::rollback();
            if (File::exists($file_name_tmp)) {
                File::delete($file_name_tmp);
            }
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
        } catch (\Throwable $e) {
            DB::rollback();
            if (File::exists($file_name_tmp)) {
                File::delete($file_name_tmp);
            }
            return response()->json([  'message' => $e->getMessage()], 500,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
        }    
    }

    public function detail_object_attribute_get(Request $request, $detail_object_id)
    {
        $tmpDir = ""; 
        try{
            //項目チェック
            //$detail_object_idのチェック 

            if(!is_numeric($detail_object_id)) {
                throw new \Exception('Parameter error! detail_object_id not numeric');
            } 
            //グループIDチェック
            $masterResult = DetailObjectMaster::find(
                $detail_object_id
            );

            if($masterResult == null) {
                throw new \Exception('Parameter error! wrong detail_object_id ');
            } 
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        } 

        try {
            //一次的ディレクトリ作成
            $tmpDir = storage_path('app/public/detailObjectAttributeGet/') . uniqid() . '/'; 
            if (!is_dir($tmpDir)) {
                mkdir($tmpDir, 0777, true);
            }
            
            //Zip圧縮しながらstreamでのdowmload開始
            $zips = new ZipStream\ZipStream(
                outputName: '',
                // enable output of HTTP headers
                sendHttpHeaders: true ,
                contentDisposition:'attachment; filename=detail_object_attribute_' . $detail_object_id . '.zip' ,
                contentType: 'application/zip'
            ); 

            $selectResults = SpacialDetailAttributeFile::where([
                ['detail_object_id', '=', $detail_object_id]
            ])->get();
        
            $zipname = $detail_object_id . '.zip';
            $zip_spatial = new \ZipArchive();
            $zip_spatial->open($tmpDir.$zipname, \ZipArchive::CREATE);
            $filename_csv = $detail_object_id . '.csv';
            $csvTmp = "";
            $zip_exist = false;
            foreach ($selectResults as $selectRes) {
                $fileName = $selectRes->attribute_file_name;
                $fileNameFull = str_replace(config('userapi.driveFrom') , config('userapi.driveTo') , $selectRes->attribute_file_path);
                if (File::exists($fileNameFull)) {
                    $zip_spatial->addFile($fileNameFull, $fileName); 
                    $zip_exist = true;
                }
                //csvファイル作成とzipに追加      
                $csvTmp = $csvTmp . $selectRes->detail_object_id      .  "," .	
                $selectRes->id	.  "," .
                $selectRes->attribute_file_name	.  "," .
                $selectRes->attribute_info . "\n"
                ;             
            }
       
            $zip_spatial->close();
            if ($zip_exist) {
                $zips->addFileFromPath(
                    fileName: $zipname,
                    path: $tmpDir.$zipname,
                );
            }
            $zips->addFile(fileName: $filename_csv , data: $csvTmp);           
            //zipsteamの終了
            $zips->finish();
            if (is_dir($tmpDir)) {
                File::deleteDirectory($tmpDir);
            }

        //例外処理開始    
        } catch (\Throwable $e) {
            if (is_dir($tmpDir)) {
                File::deleteDirectory($tmpDir);
            }
            return response()->json([  'message' => $e->getMessage()], 500,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }    
    }

    public function detail_object_attribute_updt(Request $request, $detail_object_id, $detail_object_attribute_id)
    {
        $file_name_tmp = "";
        $file_flg = false;
        try {
            //$detail_object_idのチェック    
            if(!is_numeric($detail_object_id)) {
                throw new \Exception('Parameter error! detail_object_id not numeric');
            } 
            //$detail_object_attribute_idのチェック    
            if(!is_numeric($detail_object_attribute_id)) {
                throw new \Exception('Parameter error! detail_object_attribute_id not numeric');
            } 

            //ファイルが複数だとエラー     
            if($request->has('attribute_file')) {
                $attribute_file = $request->file("attribute_file");
                if (is_array($attribute_file)) {
                    throw new \Exception('Parameter error! too many attribute_files');
                }
                if (!$attribute_file == null) {
                    $file_flg = true;
                }
            }
            //グループIDチェック
            $masterResult = DetailObjectMaster::find(
                $detail_object_id
            );

            if($masterResult == null) {
                throw new \Exception('Parameter error! wrong detail_object_id ');
            } 

            $findResult = SpacialDetailAttributeFile::find($detail_object_attribute_id);
            if($findResult == null) {
                throw new \Exception('Parameter error! wrong detail_object_attribute_id  ');
            } 
            if($findResult->detail_object_id != $detail_object_id) {
                throw new \Exception('Parameter error! wrong detail_object_id or detail_object_attribute_id  ');
            }    
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }   

        try {
            $attribute_file_name = $findResult->attribute_file_name;
            $attribute_file_path = $findResult->attribute_file_path;
            if ($file_flg == true) {
                $copyTo = config('userapi.detailpath') . '/' . $detail_object_id . '/attribute/';
                if (!is_dir($copyTo)) {
                    mkdir($copyTo, 0777, true);
                }
                //所定の場所にcopy
                if (File::exists($findResult->attribute_file_path)) {
                    File::delete($findResult->attribute_file_path);
                }    
                $file_name_tmp = $attribute_file->store('public');
                $file_name_tmp = storage_path('app/') . $file_name_tmp;
                if ($attribute_file != null) {
                    $attribute_file_name = $attribute_file->getClientOriginalName();
                    $attribute_file_path = $copyTo.$attribute_file_name;
                    File::copy( $file_name_tmp , $attribute_file_path);
                }
            }
            //attribute_infoの設定がされていなければ更新前を代入
            if(!$request->has('attribute_info') ) {
                $attribute_info = $findResult->attribute_info;
            } else {
                $attribute_info = $request->attribute_info;   
            }   

            $findResult->update([  
                "attribute_file_name"  => $attribute_file_name,
                "attribute_file_path"  => $attribute_file_path,
                "attribute_info"       => $attribute_info,
                "update_memo"          => date('Y-m-d H:i:s') . "," .  $findResult->update_memo
            ]);
            if (File::exists($file_name_tmp)) {
                File::delete($file_name_tmp);
            } 
        } catch (\Throwable $e) {
            if (File::exists($file_name_tmp)) {
                File::delete($file_name_tmp);
            } 
            return response()->json([  'message' => $e->getMessage()], 500,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }       
    }

    public function detail_object_attribute_del(Request $request, $detail_object_id, $detail_object_attribute_id)
    {
        try {
            //項目チェック
            //$detail_object_idのチェック  
            if(!is_numeric($detail_object_id)) {
                throw new \Exception('Parameter error! detail_object_id not numeric');
            } 

            if(!is_numeric($detail_object_attribute_id)) {
                throw new \Exception('Parameter error! detail_object_attribute_id not numeric');
            } 
            //$detail_object_id存在チェック
            $masterResult = DetailObjectMaster::find(
                $detail_object_id
            );

            if($masterResult == null) {
                throw new \Exception('Parameter error! wrong detail_object_id ');
            }

            $findResult = SpacialDetailAttributeFile::find($detail_object_attribute_id);

            if($findResult == null) {
                throw new \Exception('Parameter error! wrong detail_object_attribute_id ');
            } 
            if($findResult->detail_object_id != $detail_object_id) {
                throw new \Exception('Parameter error! wrong detail_object_id or detail_object_attribute_id  ');
            } 
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }           
        try {
            $findResult->delete();
        } catch (\Throwable $e) {
            return response()->json([  'message' => $e->getMessage()], 500,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
            );
        }      
        
    }

   
}
