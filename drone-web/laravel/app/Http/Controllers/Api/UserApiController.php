<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Http\Controllers\Api\ApiFunction;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Log;
use Illuminate\Support\Facades\Storage;
use App\Models\SpaceObject;
use ZipStream;

class UserApiController extends Controller
{
    
    public function vertex_points_on_spatial_id(Request $request)
    {   
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
        if ( $request["crs"] == null ) {
            $crs = '4326';
        } else {  
            $crs = $request["crs"];
        }
        if(!is_numeric($crs)) {
            return response()->json([  'message' => 'Parameter error! crs not numeric'], 400,
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
        //spatial_idsの要素数だけC++アプリを実行
        foreach($tmp_request as $key => $val) {
            //空間IDのフォーマットチェック 
            $return_cd = ApiFunction::check_spatialId($val);
            if( $return_cd == 99 ) {
                return response()->json([  'message' => "Parameter error! " . $val . " is wrong spatial_id" ], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                   JSON_UNESCAPED_UNICODE
                    );
            }
            $sap = ['spatial_id' => $val];           
            $points = shell_exec( config('userapi.exepath') . ' 1 '  . $val . ' ' . $crs);
            //エラーで戻ってきた時の処理
            if($points == null || $points == false) {
                return response()->json([  'message' => 'Parameter or server error!'], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
            }
            if($points == 'wrong parameters!') {
                return response()->json([  'message' => 'Parameter error!'], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
            }
            //$poins(空白1文字で区切られている８つの頂点座標)を配列に追加    
            $sap['points'] = $points;
            $response_out[] = $sap;
            
        }

        return response()->json(
           $response_out,200,['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
        );
    }

    public function center_point_on_spatial_id(Request $request)
    {
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
        if ( $request["crs"] == null ) {
            $crs = '4326';
        } else {  
            $crs = $request["crs"];
        }
        //crsの数値チェック 
        if(!is_numeric($crs)) {
            return response()->json([  'message' => 'Parameter error! crs not numeric'], 400,
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
        //spatial_idsの要素数だけC++アプリを実行
        foreach($tmp_request as $key => $val) {
            
            //空間IDのフォーマットチェック 
            $return_cd = ApiFunction::check_spatialId($val);
            if( $return_cd == 99 ) {
                return response()->json([  'message' => "Parameter error! " . $val . " is wrong spatial_id" ], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                   JSON_UNESCAPED_UNICODE
                    );
            }
            $sap = ['spatial_id' => $val];           
            $points = shell_exec( config('userapi.exepath') . ' 4 '  . $val . ' ' . $crs);
            //エラーで戻ってきた時の処理
            if($points == null || $points == false) {
                return response()->json([  'message' => 'Parameter or server error!'], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
            }

            if($points == 'wrong parameters!') {
                return response()->json([  'message' => 'Parameter error!'], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
            }
            //$poins(空白1文字で区切られている中心座標)を配列に追加 
            $sap['points'] = $points;
            $response_out[] = $sap;
            //pclose($points);
        }

        return response()->json(
           $response_out,200,['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
        );
    }

    public function spatial_ids_adjacent_to_faces(Request $request)
    {
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
        //spatial_idsの要素数だけC++アプリを実行
        foreach($tmp_request as $key => $val) {
            
            //空間IDのフォーマットチェック 
            $return_cd = ApiFunction::check_spatialId($val);
            if( $return_cd == 99 ) {
                return response()->json([  'message' => "Parameter error! " . $val . " is wrong spatial_id" ], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                   JSON_UNESCAPED_UNICODE
                );
            }
            $sap = ['spatial_id' => $val];           
            $return = shell_exec( config('userapi.exepath') . ' 5 '  . $val );
            //エラーで戻ってきた時の処理
            if($return == null || $return == false) {
                return response()->json([  'message' => 'Parameter or server error!'], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
            }
            if($return == 'wrong parameters!') {
                return response()->json([  'message' => 'Parameter error!'], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
            }
            //$return(":"で区切られている6個の空間ID)を連想配列にして更に配列に追加 
            $spatialIds = explode(":", $return);
            $sap['spatial_ids'] = $spatialIds;
            $response_out[] = $sap;
            //pclose($points);
        }

        return response()->json(
           $response_out,200,['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
        );
    }

    public function spatial_ids_around_voxel(Request $request)
    {
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
        //spatial_idsの要素数だけC++アプリを実行
        foreach($tmp_request as $key => $val) {
            //空間IDのフォーマットチェック 
            $return_cd = ApiFunction::check_spatialId($val);
            if( $return_cd == 99 ) {
                return response()->json([  'message' => "Parameter error! " . $val . " is wrong spatial_id" ], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                   JSON_UNESCAPED_UNICODE
                );
            }
            $sap = ['spatial_id' => $val];           
            $return = shell_exec( config('userapi.exepath') . ' 6 '  . $val );
            //エラーで戻ってきた時の処理
            if($return == null || $return == false) {
                return response()->json([  'message' => 'Parameter or server error!'], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
            }
            if($return == 'wrong parameters!') {
                return response()->json([  'message' => 'Parameter error!'], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
            }
            //$return(":"で区切られている26個の空間ID)を連想配列にして更に配列に追加 
            $spatialIds = explode(":", $return);
            $sap['spatial_ids'] = $spatialIds;
            $response_out[] = $sap;
        }

        return response()->json(
           $response_out,200,['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
        );
    }

    public function spatial_ids_cylinders(Request $request)
    {
        //座標数の上限数
        $points_max = 100;
        //リクエストjsonのチェック
        if(json_last_error() !== JSON_ERROR_NONE) {
            return response()->json([  'message' => 'Json syntax error!'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
        if($request["points"] == null) {
            return response()->json([  'message' => 'Parameter error! no points'], 400,
        ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        } 
        if($request["radius"] == null) {
            return response()->json([  'message' => 'Parameter error! no radius'], 400,
        ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        }   
        //区切り空白が2文字以上続くとエラー
        if (strpos($request["points"], "  ")) {
            return response()->json([  'message' => 'Parameter error! too long spaces in points.'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
               JSON_UNESCAPED_UNICODE
                );

        }
        //空白文字で区切られている座標の数が6個以上でかつ3で割り切れないとエラー
        $repPoints = str_replace(' ' , ':' , $request["points"], $repCount);
        if(($repCount < 5) || (($repCount + 1) % 3 != 0)) {
            return response()->json([  'message' => 'Parameter error! wrong points'], 400,
        ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        }
        //座標数が上限を超えたらエラー
        if(($repCount + 1) / 3 > $points_max) {
            return response()->json([  'message' => 'Parameter error! points number must be less than ' .  $points_max + 1 . '.'], 400,
        ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        }
        $radius =  $request["radius"];
        if(!is_numeric($radius)) {
            return response()->json([  'message' => 'Parameter error! radius not numeric'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        } 
        if ( $request["crs"] == null ) {
            $crs = '4326';
        } else {  
            $crs = $request["crs"];
        }
        if(!is_numeric($crs)) {
            return response()->json([  'message' => 'Parameter error! crs not numeric'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        } 
        if ( $request["zoom"] == null ) {
            $zoom = '17';
        } else {  
            $zoom = $request["zoom"];
        }
        if(!is_numeric($zoom)) {
            return response()->json([  'message' => 'Parameter error! zoom not numeric'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        } 
        //C++アプリを実行
        $return = shell_exec( config('userapi.exepath') . ' 2 ' . $repPoints . ' ' . $radius . ' ' . $crs . ' ' . $zoom);
        //エラーで戻ってきた時の処理
        if($return == 'wrong parameters!') {
            return response()->json([  'message' => 'Parameter error!'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
        if($return == null || $return == false) {
            return response()->json([  'message' => 'Parameter or server error!'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
        //$return(":"で区切られている空間ID)を配列にする 
        $spatialId = explode(":", $return);
        return response()->json(
            [ 'spatial_ids' => $spatialId ], 200 ,['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
         );
       
    }

    public function spatial_ids_on_points(Request $request)
    {
         //座標数の上限数
        $points_max = 100;
        //リクエストjsonのチェック
        if(json_last_error() !== JSON_ERROR_NONE) {
            return response()->json([  'message' => 'Json syntax error!'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
        if($request["points"] == null) {
            return response()->json([  'message' => 'Parameter error! no points'], 400,
        ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        }  
        //区切り空白が2文字以上続くとエラー
        if (strpos($request["points"], "  ")) {
            return response()->json([  'message' => 'Parameter error! too long spaces in points.'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
               JSON_UNESCAPED_UNICODE
                );

        }
        //空白区切りを":"区切りに変更
        $repPoints = str_replace(" ", ":" , $request["points"], $repCount);
        //空白文字で区切られている座標の数が3で割り切れないとエラー
        if(($repCount + 1) % 3 != 0) {
            return response()->json([  'message' => 'Parameter error! wrong points numbers.'], 400,
        ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        }
        //座標数が上限を超えたらエラー
        if(($repCount + 1) / 3 > $points_max) {
            return response()->json([  'message' => 'Parameter error! points number must be less than ' .  $points_max + 1 . '.'], 400,
        ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        }

        if ( $request["crs"] == null ) {
            $crs = '4326';
        } else {  
            $crs = $request["crs"];
        }
        if(!is_numeric($crs)) {
            return response()->json([  'message' => 'Parameter error! crs not numeric'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        } 
        if ( $request["zoom"] == null ) {
            $zoom = '17';
        } else {  
            $zoom = $request["zoom"];
        }
        if(!is_numeric($zoom)) {
            return response()->json([  'message' => 'Parameter error! zoom not numeric'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        } 
        //C++アプリを実行
        $return = shell_exec( config('userapi.exepath') . ' 3 ' . $repPoints  . ' ' . $crs . ' ' . $zoom);

        if($return == 'wrong parameters!') {
            return response()->json([  'message' => 'Parameter error!'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
        //エラーで戻ってきた時の処理
        if($return == null || $return == false) {
            return response()->json([  'message' => 'Parameter or server error!'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
        //$return(":"で区切られている空間ID)を配列にする 
        $spatialIds = explode(":", $return);
        return response()->json(
            [ 'spatial_ids' => $spatialIds ], 200 ,['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
         );
       
    }

    public function parent_spatial_id(Request $request)
    {
        //リクエストjsonのチェック
        if(json_last_error() !== JSON_ERROR_NONE) {
            return response()->json([  'message' => 'Json syntax error!'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
        //spatial_idが配列だとエラー
        if( ($request["spatial_id"] == null) || (is_array($request["spatial_id"]) == true) ) {
            return response()->json([  'message' => 'Parameter error! wrong spatila_id'], 400,
        ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        } 
        //空間IDのフォーマットチェック  
        $return_cd = ApiFunction::check_spatialId($request["spatial_id"]);
        if( $return_cd == 99 ) {
            return response()->json([  'message' => "Parameter error! " . $request["spatial_id"] . " is wrong spatial_id" ], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
               JSON_UNESCAPED_UNICODE
            );
        }
        $spatialId_arr = explode("/", $request["spatial_id"]);
        $level = $spatialId_arr[0] - 1;
        $Zpoint = floor($spatialId_arr[1] /2);
        $Xpoint = floor($spatialId_arr[2] /2);
        $Ypoint = floor($spatialId_arr[3] /2);

        $return_spatialId = $level . '/' . $Zpoint . '/' . $Xpoint . '/' . $Ypoint;
        return response()->json(
        //     $response_out,200,['Content-Type' => 'application/json;charset=UTF-8'],
            [ 'spatial_id' => $return_spatialId ], 200 ,['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
         );      
    }

    public function child_spatial_ids(Request $request)
    {
        //リクエストjsonのチェック
        if(json_last_error() !== JSON_ERROR_NONE) {
            return response()->json([  'message' => 'Json syntax error!'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
        //spatial_idが配列だとエラー
        if( ($request["spatial_id"] == null) || (is_array($request["spatial_id"]) == true) ) {
            return response()->json([  'message' => 'Parameter error! wrong spatila_id'], 400,
        ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        }
        
        //空間IDのフォーマットチェック 
        $return_cd = ApiFunction::check_spatialId($request["spatial_id"]);
        if( $return_cd == 99 ) {
            return response()->json([  'message' => "Parameter error! " . $request["spatial_id"] . " is wrong spatial_id" ], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
               JSON_UNESCAPED_UNICODE
            );
        }   
        $spatialId_arr = explode("/", $request["spatial_id"]);
        $level  = $spatialId_arr[0] + 1;
        $Zpoint = $spatialId_arr[1] * 2;
        $Xpoint = $spatialId_arr[2] * 2;
        $Ypoint = $spatialId_arr[3] * 2;

        for($iz = 0; $iz < 2; ++$iz){
            for($ix = 0; $ix < 2; ++$ix){
                for($iy = 0; $iy < 2; ++$iy){
                        $return_spatialIds[] = $level . '/' . $Zpoint + $iz . '/' . $Xpoint + $ix . '/' . $Ypoint + $iy;
                }
            }
        }

        return response()->json(
            [ 'spatial_ids' => $return_spatialIds ], 200 ,['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
         );
       
    }

    public function point_cloud_file(Request $request)
    {
        //リクエストjsonのチェック
        if(json_last_error() !== JSON_ERROR_NONE) {
            return response()->json([  'message' => 'Json syntax error!'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
        if($request["type_cd"] == null) {
            return response()->json([  'message' => 'Parameter error! no type_cd'], 400,
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
        $typeCd = $request["type_cd"];
        if(!is_numeric($typeCd)) {
            return response()->json([  'message' => 'Parameter error! type_cd not numeric'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        } 
        //Zip圧縮しながらstreamでのdowmload開始
        $zips = new ZipStream\ZipStream(
            outputName: '',
            // enable output of HTTP headers
            sendHttpHeaders: true ,
            contentDisposition:'attachment; filename=point_cloud_file.zip' ,
            contentType: 'application/zip'
        );
        //spatial_idsが配列でない時は配列に変換
        if (is_array($request["spatial_ids"])) {
            $tmp_request = $request["spatial_ids"];
        } else {
            $tmp_request = array($request["spatial_ids"]);
        }

        $fileCount = 0;
        //spatial_idsの要素数だけDB検索を実行
        foreach($tmp_request as $key => $val) {
            //空間IDのフォーマットチェック 
            $return_cd = ApiFunction::check_spatialId($val);
            if( $return_cd != 17 ) {
                return response()->json([  'message' => "Parameter error! " . $val . " is wrong spatial_id" ], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                   JSON_UNESCAPED_UNICODE
                );
            }    
            $selectRes = SpaceObject::where([
                ['object_cd', '=', $typeCd],
                ['spatial_id', '=', $val]
                //作成日が一番大きいレコードを採用
              ])->orderBy('created_at', 'desc')
                ->first();
            if ($selectRes == null)  {
                continue;
            }
            $fileCount++;
            $tmpFile = str_replace(config('userapi.driveFrom') , config('userapi.driveTo') , $selectRes->point_cloud_file_path);
            $file = str_replace( '\\' , '/' , $tmpFile);
            $file_info = pathinfo($file);
		    $file_name = $file_info['filename'].'.'.$file_info['extension'];
            //$time_start = microtime(true);  
            $zips->addFileFromPath(
                fileName: $file_name,
                path: $file,
             );
                  
            //$time = microtime(true) - $time_start;
            //Log::info($time . " seconds");

        }
        //検索結果が何もなかった場合の処理
        if ($fileCount == 0) {
            $zips->addFile(fileName: 'nothing.txt', data: 'Nothing was selected.');
        }  
        //zipsteamの終了
        $zips->finish();
 
    }

    public function spatial_voxel(Request $request)
    {
        //リクエストjsonのチェック
        if(json_last_error() !== JSON_ERROR_NONE) {
            return response()->json([  'message' => 'Json syntax error!'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
            JSON_UNESCAPED_UNICODE
            );
        }
        if($request["type_cd"] == null) {
            return response()->json([  'message' => 'Parameter error! no type_cd'], 400,
        ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        } 
        if($request["spatial_ids"] == null) {
            return response()->json([  'message' => 'Parameter error! no spatila_ids'], 400,
        ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        }  
 

        $typeCd = $request["type_cd"];
        if(!is_numeric($typeCd)) {
            return response()->json([  'message' => 'Parameter error! type_cd not numeric'], 400,
            ['Content-Type' => 'application/json;charset=UTF-8'],
           JSON_UNESCAPED_UNICODE
            );
        } 
        //指定日付の時刻は最大時刻をセット
        if ( $request["date"] == null ) {
            $reqDate = '9999-12-31 23:59:59';
        } else { 
            //日付のフォーマットチェック 
            $YYYYMMDD = explode('-', $request["date"]);
            if(count($YYYYMMDD) == 3) {
                if(!checkdate($YYYYMMDD[1], $YYYYMMDD[2], $YYYYMMDD[0])){ 
                    return response()->json([  'message' => 'Parameter error! wrong date'], 400,
                    ['Content-Type' => 'application/json;charset=UTF-8'],
                    JSON_UNESCAPED_UNICODE
                    );
                }
            } else {             
                return response()->json([  'message' => 'Parameter error! wrong date'], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                JSON_UNESCAPED_UNICODE
                );
            }
            $reqDate = $request["date"] . ' 23:59:59';
        }
        //Zip圧縮しながらstreamでのdowmload開始
        $zips = new ZipStream\ZipStream(
            outputName: '',
         
            // enable output of HTTP headers
            sendHttpHeaders: true ,
            contentDisposition:'attachment; filename=spatial_voxel.zip' ,
            contentType: 'application/zip'

        );

        if (is_array($request["spatial_ids"])) {
            $tmp_request = $request["spatial_ids"];
        } else {
            $tmp_request = array($request["spatial_ids"]);
        }
       
        $fileCount = 0;
        $jsonArr2 = [];
        //spatial_idsの要素数だけDB検索を実行
        foreach($tmp_request as $key => $val) {
            //空間IDのフォーマットチェック 
            $return_cd = ApiFunction::check_spatialId($val);
            if( $return_cd != 17 ) {
                return response()->json([  'message' => "Parameter error! " . $val . " is wrong spatial_id" ], 400,
                ['Content-Type' => 'application/json;charset=UTF-8'],
                   JSON_UNESCAPED_UNICODE
                );
            } 
            if ($reqDate == '9999-12-31 23:59:59') {   
                $selectRes = SpaceObject::where([
                    ['object_cd', '=', $typeCd],
                    ['spatial_id', '=', $val],
                    ['created_at', '<=', $reqDate]
                    //作成日が一番大きいレコードを採用
                ])->orderBy('created_at', 'desc')
                    ->first();
            } else {
                $selectRes = SpaceObject::withTrashed()->where([
                    ['object_cd', '=', $typeCd],
                    ['spatial_id', '=', $val],
                    ['created_at', '<=', $reqDate]
                    //作成日が一番大きいレコードを採用
                ])->orderBy('created_at', 'desc')
                    ->first();
            }
                
            if ($selectRes == null)  {
                    continue;
            }

            $fileCount++;
            $tmpFile = str_replace(config('userapi.driveFrom') , config('userapi.driveTo') , $selectRes->voxel_bit_file_path);
            $file = str_replace( '\\' , '/' , $tmpFile);
           
            $file_info = pathinfo($file);
		    $file_name = $file_info['filename'].'.'.$file_info['extension'];

            $zips->addFileFromPath(
                fileName: $file_name,
                path: $file,
             );
            //csvファイルの編集とzipへの追加      
            $csvTmp =   $selectRes->spatial_id      .  "," .	
                        $selectRes->north_latitude	.  "," .
                        $selectRes->south_latitude	.  "," .
                        $selectRes->east_longitude	.  "," .
                        $selectRes->west_longitude	.  "," .
                        $selectRes->lower_altitude	.  "," .
                        $selectRes->upper_altitude	.  "," .
                        $selectRes->object_cd	    .  "," .
                        $selectRes->point_cloud_epsg	    .  "," .
                        $selectRes->hash_18	        .  "," .
                        $selectRes->hash_19	        .  "," .
                        $selectRes->hash_20	        .  "," .
                        $selectRes->hash_21	        .  "," .
                        $selectRes->hash_22	        .  "," .
                        $selectRes->hash_23	        .  "," .
                        $selectRes->hash_24	        .  "," .
                        $selectRes->hash_25	        .  "," .
                        $selectRes->hash_26	        .  "," .
                        $selectRes->hash_27	        .  "," .
                        $selectRes->hash_28	        .  "," .
                        $selectRes->created_at	
            ;
            $zips->addFile(fileName: $file_info['filename'] . '.csv' , data: $csvTmp);  
            //json用配列の編集
            $jsonArr1 = ['spatial_id' => $selectRes->spatial_id ];    
            $jsonArr1['voxel_bit_file_name'] = $file_name;
            $jsonArr1['voxel_bit_spatial_zoom_level'] = $selectRes->voxel_bit_spatial_zoom_level;
            $jsonArr1['north_latitude'] = $selectRes->north_latitude;
            $jsonArr1['south_latitude'] = $selectRes->south_latitude;
            $jsonArr1['east_longitude'] = $selectRes->east_longitude; 
            $jsonArr1['west_longitude'] = $selectRes->west_longitude; 
            $jsonArr1['lower_altitude'] = $selectRes->lower_altitude;
            $jsonArr1['upper_altitude'] = $selectRes->upper_altitude; 
            $jsonArr1['object_cd'] = $selectRes->object_cd;  
            $jsonArr1['point_cloud_epsg'] = $selectRes->point_cloud_epsg; 
            $jsonArr1['hash_18'] = $selectRes->hash_18;   
            $jsonArr1['hash_19'] = $selectRes->hash_19; 
            $jsonArr1['hash_20'] = $selectRes->hash_20;
            $jsonArr1['hash_21'] = $selectRes->hash_21;
            $jsonArr1['hash_22'] = $selectRes->hash_22;
            $jsonArr1['hash_23'] = $selectRes->hash_23;
            $jsonArr1['hash_24'] = $selectRes->hash_24;
            $jsonArr1['hash_25'] = $selectRes->hash_25;
            $jsonArr1['hash_26'] = $selectRes->hash_26;
            $jsonArr1['hash_27'] = $selectRes->hash_27;
            $jsonArr1['hash_28'] = $selectRes->hash_28;
            $jsonArr1['created_at'] = $selectRes->created_at;  
            $jsonArr2[] = $jsonArr1;          
        } 
        $jsonArr3 = [ 'laz_files' => $jsonArr2 ]; 
        $response_json = json_encode(['voxel_info' => $jsonArr3], JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
        $zips->addFile(fileName: 'info.json' , data: $response_json);     

        //検索結果が何もなかった場合の処理        
        //if ($fileCount == 0) {
        //    $zips->addFile(fileName: 'nothing.txt', data: 'Nothing was selected.');
        //}  

        $zips->finish();
             
    }
}
