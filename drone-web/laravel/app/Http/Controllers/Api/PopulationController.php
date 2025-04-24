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
use Illuminate\Support\Facades\Http;
use App\Models\AveragePopulation;

class PopulationController extends Controller
{   
    
    public static function get_average_population($request)
    {
        //項目チェック
        if(!$request->has('other') ) {
            throw new \Exception('Parameter error! no [other]');
        }
        //高さは何が入力されても0に置換する
        $spatialId_arr = explode("/", $request["identification"]); 
        $spatial_id = $spatialId_arr[0]."/0/".$spatialId_arr[2]."/".$spatialId_arr[3];
    
        //holiday_flgチェック
        if(!isset($request->other['holidayFlg'])) {
            throw new \Exception('Parameter error! no [holidayFlg].');
        }
        if($request->other['holidayFlg'] == null) {
            throw new \Exception('Parameter error! no [holidayFlg]');
        }     
        if( ($request->other["holidayFlg"] != '0') && ($request->other["holidayFlg"] != '1')) {
            throw new \Exception('Parameter error! holidayFlg must be 0 or 1');
        }
        /*
        if($request->other['hour'] == null) {
            throw new \Exception('Parameter error! no [hour]');
        } 
        */    
        if(!isset($request->other['hour'])) {
            throw new \Exception('Parameter error! no [hour].');
        }
        if(!is_numeric($request->other['hour'])) {
            throw new \Exception('Parameter error! hour not numeric');
        }    
        if(($request->other['hour'] < 0) || ($request->other['hour'] > 23)) {
            throw new \Exception('Parameter error! hour must be 0 - 23');
            } 
        //$spatial_arr = PopulationController::calc_spatial_area($spatial_id);
        
        $stay_result = 0.0;
        $move_result = 0.0;
        $selectResults = AveragePopulation::where([
            ['spatial_id', '=', $spatial_id],
            ['from_datetime', '<=', $request->timing],
            ['to_datetime',   '>=', $request->timing],
            ['hour', '=', $request->other["hour"]],
            ['holiday_flg', '=', $request->other["holidayFlg"]]
            ])
            ->orderBy('mesh_area', 'asc')
            ->orderBy('from_datetime', 'desc')->get(); 
        $work = ['spatialId' => $spatial_id];
        $objects = [];
        $timing = [];
        $response_tmp2 = [];
        $wk_mesh_area = "";
        foreach ($selectResults as $selectResult) {
            if ($wk_mesh_area != $selectResult->mesh_area) {
                /*
                $response_tmp =  ['mesh_area' => $selectResult->mesh_area ]; 
                $response_tmp['city_code']   = $selectResult->city_code;
                $response_tmp['stay_average_population']  = $selectResult->stay_average_population;
                $response_tmp['move_average_population']  = $selectResult->move_average_population;
                $response_tmp2[] = $response_tmp;
                $area_ratio = PopulationController::calc_area_ratio($selectResult->mesh_area, $spatial_arr);
                */
                $stay_result += $selectResult->stay_average_population_spatial ;
                $move_result += $selectResult->move_average_population_spatial ;
                $timing['fromDatetime'] = $selectResult->from_datetime; 
                $timing['endDatetime'] = $selectResult->to_datetime;
                $wk_mesh_area = $selectResult->mesh_area; 
            }
 
        }
        $work['timing'] = $timing; 
        //$other['src'] = $response_tmp2;
        $response_tmp3 =  ['stayAveragePopulation' => (double)number_format($stay_result, 2) ];
        $response_tmp3['moveAveragePopulation'] =  (double)number_format($move_result, 2);
        //$other['result'] = $response_tmp3;
        $work['other'] = $response_tmp3; 
        $objects[] = $work;
        $objects_res = ['objects' => $objects]; 
        return $objects_res;
        
    }

    public static function calc_spatial_area($spatial_id)
    {
        $crs = '4326';
        $points = shell_exec( config('userapi.exepath') . ' 1 '  . $spatial_id . ' ' . $crs);
            //エラーで戻ってきた時の処理
            if($points == null || $points == false) {
                throw new \Exception('Parameter or server error!');
            }
            if($points == 'wrong parameters!') {
                throw new \Exception('Parameter error!');
            }
            //$poins(空白1文字で区切られている８つの頂点座標)で配列を作成    
            $points_arr = explode(" ", $points);
            //面積比計算に必要な座標と面積を配列に追加してリターンする
            $return_arr[0] = $points_arr[9];
            $return_arr[1] = $points_arr[10];
            $return_arr[2] = $points_arr[0];
            $return_arr[3] = $points_arr[7];
            $return_arr[4] = ($points_arr[0] - $points_arr[9]) * ($points_arr[7] - $points_arr[10]);
            Log::info(strval($return_arr[4]) . "=" . strval($points_arr[0]) . "-" . strval($points_arr[9]) . "*" .   strval($points_arr[7]) . "-" . strval($points_arr[10]) );

            return $return_arr;
    }

    public static function calc_area_ratio($mesh5, $spatial_arr)
    {
        $x1 =   (int)substr($mesh5,0,2);
        $y1 =   (int)substr($mesh5,2,2);
        $x2 =   (int)substr($mesh5,4,1);
        $y2 =   (int)substr($mesh5,5,1);
        $x3 =   (int)substr($mesh5,6,1);
        $y3 =   (int)substr($mesh5,7,1);
        $xy4 =  (int)substr($mesh5,8,1);
        //$xy5 =  (int)substr($mesh5,9,1);

        $lat1 = $x1 * 2/3 ;
        $lon1 = $y1 + 100.0 ;

        $lat2 = $x2 * 1/12 ;
        $lon2 = $y2 * 1/8 ;

        $lat3 = $x3 * 1/120 ;
        $lon3 = $y3 * 1/80 ;
        
        $x4 = 0;
        $y4 = 0;
        if ($xy4 == 2) {
            $y4 = 1;
        }
        if ($xy4 == 3) {
            $x4 = 1;
        }
        if ($xy4 == 4) {
            $x4 = 1;
            $y4 = 1;
        }
        $lat4 = $x4 * 1/240;
        $lon4 = $y4 * 1/160 ;

        //mesh4の南西端の座標
        $swLatM = $lat1 + $lat2 + $lat3 + $lat4 ;
        $swLonM = $lon1 + $lon2 + $lon3 + $lon4 ;
        //mesh4の北西端の座標
        $nwLatM = $swLatM + 1/240 ;
        //$nwLonM = $swLon ;
        //mesh4の南東端の座標
        //$seLatM = $swLat ;
        $seLonM = $swLonM + 1/160 ;

        /*
        $x5 = 0;
        $y5 = 0;
        if ($xy5 == 2) {
            $y5 = 1;
        }
        if ($xy5 == 3) {
            $x5 = 1;
        }
        if ($xy5 == 4) {
            $x5 = 1;
            $y5 = 1;
        }
        $lat5 = $x5 * 1/480 ;
        $lon5 = $y5 * 1/320 ;

        //mesh5の南西端の座標
        $swLatM = $lat1 + $lat2 + $lat3 + $lat4 + $lat5 ;
        $swLonM = $lon1 + $lon2 + $lon3 + $lon4 + $lon5 ;
        //mesh5の北西端の座標
        $nwLatM = $swLatM + 1/480 ;
        //$nwLonM = $swLon ;
        //mesh5の南東端の座標
        //$seLatM = $swLat ;
        $seLonM = $swLonM + 1/320 ;
        */
        //空間IDの南西端の座標
        $swLatS = $spatial_arr[0];
        $swLonS = $spatial_arr[1];
        //空間IDの北西端の座標
        $nwLatS = $spatial_arr[2];
        //$nwLonS = $spatial_arr[1];
        //空間IDの南東端の座標
        //$seLatS = $spatial_arr[0];
        $seLonS = $spatial_arr[3];
        
        //緯度でmeshと空間IDで重なっている面積を計算
        $minLatMS = $swLatS;
        if($swLatS < $swLatM) {
            $minLatMS = $swLatM;
        }
        $maxLatMS = $nwLatS;
        if($nwLatS > $nwLatM) {
            $maxLatMS = $nwLatM;
        }
        //経度でmeshと空間IDで重なっている面積を計算
        $minLonMS = $swLonS;
        if($swLonS < $swLonM) {
            $minLonMS = $swLonM;
        }
        $maxLonMS = $seLonS;
        if($seLonS > $seLonM) {
            $maxLonMS = $seLonM;
        }
        
        $areaMS = ($maxLatMS - $minLatMS) * ($maxLonMS - $minLonMS);
        $areaMesh = (1/240) * (1/160); 
        Log::info(strval($areaMS) . "=" . strval($maxLatMS) . "-" . strval($minLatMS) . "*" .   strval($maxLonMS) . "-" . strval($minLonMS) );

        Log::info("swM = " . $swLatM . "  " . $swLonM );
        Log::info("nwM = " . $nwLatM );
        Log::info("seM = " . $seLonM );

        Log::info("swS = " . $swLatS . "  " . $swLonS );
        Log::info("nwS = " . $nwLatS );
        Log::info("seS = " . $seLonS );

        //return $areaMS / $spatial_arr[4];
        return $areaMS / $areaMesh;
    }
}
