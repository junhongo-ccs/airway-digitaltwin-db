<?php

namespace App\Http\Controllers\Api;
use Illuminate\Support\Facades\Validator;
use Illuminate\Support\Facades\Log;

class ApiFunction
{
  public static function check_spatialId($spatialId)
  {
    $spatialId_arr = explode("/", $spatialId);
    if(count($spatialId_arr) != 4) {
        return 99;
    }  
    foreach($spatialId_arr as $each){
      if(!is_numeric($each)) {
        return 99;
      }
    }
    if( ($spatialId_arr[0] <= 0) || ($spatialId_arr[0] >= 36)) {
        return 99;
    }
    return $spatialId_arr[0];    
  }

  public static function edit_datetime($datetime_before)
  {
    $YYYY = substr($datetime_before,0,4); 
    $MM   = substr($datetime_before,4,2);
    $DD   = substr($datetime_before,6,2);
    $hh   = substr($datetime_before,8,2);  
    $mm   = substr($datetime_before,10,2); 
    $ss   = substr($datetime_before,12,2); 
    return $YYYY . "-" . $MM . "-" . $DD . " " . $hh . ":" . $mm . ":" . $ss;       
  }
  
  public static function check_datetime($datetime)
  {
    // 定義する日付フォーマット
    $format = 'Y-m-d H:i:s';
    // DateTimeクラスでチェック
    $d = \DateTime::createFromFormat($format, $datetime);
    // フォーマットが一致し、かつ有効な日付であるかを確認
    return $d && $d->format($format) === $datetime;
    /* 使用例
  $input = '2024-12-01 15:30:45';
  if (validateDateTime($input)) {
    echo "正しい形式です。";
  } else {
    echo "無効な形式です。";
  }
    */
  }

  public static function format_iso8601($iso8601)
  {
    $formattedDate = str_replace("T", " ", $iso8601);
    $formattedDate = preg_replace("/\.\d+Z$/", "", $formattedDate);
    return $formattedDate;
  }

  public static function div_area_into_spatialIds($nwLon,$nwLat,$seLon,$seLat)
    {
        //四隅の空間ＩＤを取得
        $zoom = 17;
        //Log::info("nwLon=".strVal($nwLon));
        //Log::info("nwLat=".strVal($nwLat));
        //Log::info("seLon=".strVal($seLon));
        //Log::info("seLat=".strVal($seLat));
        //$spatiaiId_sw = ApiFunction::get_spatial_xy_on_point($swLonM, $swLatM, $zoom);
        $spatiaiId_nw = ApiFunction::get_spatial_xy_on_point($nwLon, $nwLat, $zoom);
        $spatiaiId_se = ApiFunction::get_spatial_xy_on_point($seLon, $seLat, $zoom);
        //Log::info("spatialId_nw=".$spatiaiId_nw);
        //Log::info("spatialId_se=".$spatiaiId_se);
        //$spatiaiId_ne = ApiFunction::get_spatial_xy_on_point($neLonM, $neLatM, $zoom);
        $spatialId_arr_nw = explode("/", $spatiaiId_nw);
        $wk_min_x = (int)$spatialId_arr_nw[2];
        $wk_min_y = (int)$spatialId_arr_nw[3];
        $spatialId_arr_se = explode("/", $spatiaiId_se);
        $wk_max_x = (int)$spatialId_arr_se[2];
        $wk_max_y = (int)$spatialId_arr_se[3];
        $return_arr = array();
        for($index = $wk_min_x; $index <= $wk_max_x; $index++){
          for($indey = $wk_min_y; $indey <= $wk_max_y; $indey++){
            $wk_spatialId = (string)$zoom . "/0/" . (string)$index . "/" . (string)$indey;
            array_push($return_arr,$wk_spatialId);
          }
        }
        return $return_arr;
  }

  public static function edit_contens_on_spatial($spatial_id,$contents,$lat_start,$lon_start,$lat_end,$lon_end)
  {   //contentsのintervalとcountを計算
      $lat_count = count($contents);
      $lon_count = count($contents[0]);
      $lat_interval = ($lat_end - $lat_start) / (double)$lat_count;
      $lon_interval = ($lon_end - $lon_start) / (double)$lon_count;
      Log::info("spatial_id=".$spatial_id);
      Log::info("lat_start=".$lat_start);
      Log::info("lat_end=".$lat_end);
      Log::info("lon_start=".$lon_start);
      Log::info("lon_end=".$lon_end);
      Log::info("lat_count=".$lat_count);
      Log::info("lon_count=".$lon_count);
      Log::info("lat_interval=".$lat_interval);
      Log::info("lon_interval=".$lon_interval);

      //空間ＩＤの四隅を取得
      $spatialId_arr = explode("/", $spatial_id);
      $zoom = (int)$spatialId_arr[0];
      $lon  = (int)$spatialId_arr[2];
      $lat  = (int)$spatialId_arr[3];
      $point_arr = ApiFunction::get_vertex_on_voxel($lon, $lat, $zoom);
      $south_lat_spatial = $point_arr[0];
      $west_lon_spatial  = $point_arr[1];
      $north_lat_spatial = $point_arr[2];
      $east_lon_spatial  = $point_arr[3];

      Log::info("north_lat_spatial=".$north_lat_spatial);
      Log::info("south_lat_spatial=".$south_lat_spatial);
      Log::info("west_lon_spatial=".$west_lon_spatial);
      Log::info("east_lon_spatial=".$east_lon_spatial);

      if ($north_lat_spatial >= $lat_start) {
        $lat_index_start = 0;
      } else {
        $lat_index_start = (int)(($north_lat_spatial -  $lat_start) / $lat_interval);
      }
      if ($south_lat_spatial <= $lat_end) {
        $lat_index_end = $lat_count - 1;
      } else {
        $lat_index_end = (int)(($south_lat_spatial -  $lat_start) / $lat_interval);
      }
      if ($west_lon_spatial <= $lon_start) {
        $lon_index_start = 0;
      } else {
        $lon_index_start = (int)(($west_lon_spatial - $lon_start) / $lon_interval);
      }
      if ($east_lon_spatial >= $lon_end) {
        $lon_index_end = $lon_count - 1;
      } else {
        $lon_index_end = (int)(($east_lon_spatial - $lon_start) / $lon_interval);
      }
      Log::info("lat_index_start=".$lat_index_start);
      Log::info("lat_index_end=".$lat_index_end);
      Log::info("lon_index_start=".$lon_index_start);
      Log::info("lon_index_end=".$lon_index_end);

      $return_array = [];

      for ($i = 0; $i < $lat_count; $i++) {
        for ($j = 0; $j < $lon_count; $j++) {
          if ($i >= $lat_index_start && $i <= $lat_index_end && $j >= $lon_index_start && $j <= $lon_index_end) {
            $return_array[$i][$j] = $contents[$i][$j];
          } else {
            $return_array[$i][$j] = null;
          }
        }
      }       

      return $return_array;
}

  public static function div_area_into_spatialIds_mesh5($mesh5)
    {
        $x1 =   (int)substr($mesh5,0,2);
        $y1 =   (int)substr($mesh5,2,2);
        $x2 =   (int)substr($mesh5,4,1);
        $y2 =   (int)substr($mesh5,5,1);
        $x3 =   (int)substr($mesh5,6,1);
        $y3 =   (int)substr($mesh5,7,1);
        $xy4 =  (int)substr($mesh5,8,1);
        $xy5 =  (int)substr($mesh5,9,1);

        $lat1 = $x1 * 2/3 ;
        $lon1 = $y1 + 100.0 ;

        $lat2 = $x2 * 1/12 ;
        $lon2 = $y2 * 1/8 ;

        $lat3 = $x3 * 1/120 ;
        $lon3 = $y3 * 1/80 ;
        
        $x4 = 0;
        $y4 = 0;

        if ($xy4 < 1 || $xy4 > 4) {
          throw new \Exception('meshCode error!');
        }
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

        $x5 = 0;
        $y5 = 0;
        if ($xy5 < 1 || $xy5 > 4) {
          throw new \Exception('meshCode error!');
        }
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
        $nwLonM = $swLonM ;
        //mesh5の南東端の座標
        $seLatM = $swLatM ;
        $seLonM = $swLonM + 1/320 ;
        //mesh5の北東端の座標
        //$neLatM = $swLatM ;
        //$neLonM = $seLonM ;

        //四隅の空間ＩＤを取得
        $zoom = 17;
        //$spatiaiId_sw = ApiFunction::get_spatial_xy_on_point($swLonM, $swLatM, $zoom);
        $spatiaiId_nw = ApiFunction::get_spatial_xy_on_point($nwLonM, $nwLatM, $zoom);
        $spatiaiId_se = ApiFunction::get_spatial_xy_on_point($seLonM, $seLatM, $zoom);
        //$spatiaiId_ne = ApiFunction::get_spatial_xy_on_point($neLonM, $neLatM, $zoom);
        $spatialId_arr_nw = explode("/", $spatiaiId_nw);
        $wk_min_x = (int)$spatialId_arr_nw[2];
        $wk_min_y = (int)$spatialId_arr_nw[3];
        $spatialId_arr_se = explode("/", $spatiaiId_se);
        $wk_max_x = (int)$spatialId_arr_se[2];
        $wk_max_y = (int)$spatialId_arr_se[3];
        $return_arr = array();
        for($index = $wk_min_x; $index <= $wk_max_x; $index++){
          for($indey = $wk_min_y; $indey <= $wk_max_y; $indey++){
            $wk_spatialId = (string)$zoom . "/0/" . (string)$index . "/" . (string)$indey;
            array_push($return_arr,$wk_spatialId);
          }
        }
        return $return_arr;
  }

  public static function get_spatial_xy_on_point($lon, $lat, $zoom)
  {
    if( ($zoom <= 0) || ($zoom >= 36)) {
      throw new \Exception('zoom is out of range!');
    }
    // 経度方向の位置の計算
    // 経度に180が入力されていると位置+1の値が出力されるため、補正する
    if ($lon == 180) {
        $lon = -1.0 * $lon;
    }
    $p = (double)pow(2, $zoom);
    $x = (int)floor($p * (($lon + 180.0) / 360.0));

    // 緯度方向の位置の計算
    $y = (int)(floor($p *(1.0 - log(tan(deg2rad($lat)) +  (1.0 / cos(deg2rad($lat)))) / M_PI) / 2.0));

    return $zoom."/0/".$x."/".$y; 
  }

  public static function get_vertex_on_voxel($lon_index, $lat_index, $zoom)
  { 
    if( ($zoom <= 0) || ($zoom >= 36)) {
      throw new \Exception('zoom is out of range!');
    } 
    // 緯度の取得
    $p = (double)pow(2, $zoom);
    if ($lat_index > ((int)($p) - 1)) {
        $lat_index = (int)($p) - 1;
    } else if ($lat_index < 0) {
        $lat_index = 0;
    }

    // タイルの上辺の緯度
    $north_lat =
        rad2deg(atan(sinh(M_PI * (1.0 - 2.0 * $lat_index / $p))));

    // タイルの下辺の緯度
    $south_lat =
        rad2deg(atan(sinh(M_PI * (1.0 - 2.0 * ($lat_index + 1) / $p))));

    // 経度の取得
    if ($lon_index > ($p - 1) || $lon_index < 0) {
        // インデックスの範囲を超えている場合はn周分を無視する
        $lon_index = (int)(fmod($lon_index, $p));
    }
    $west_lon = $lon_index * (360.0 / $p) - 180.0;
    $east_lon = ($lon_index + 1) * (360.0 / $p) - 180.0;

    // 地理座標の経度・緯度の桁揃え
    $north_lat_new = ApiFunction::alignment_lonlat($west_lon, $north_lat);
    $south_lat_new = ApiFunction::alignment_lonlat($east_lon, $south_lat);

    $return_arr[0] = $south_lat_new;
    $return_arr[1] = $west_lon;
    $return_arr[2] = $north_lat_new;
    $return_arr[3] = $east_lon;

    return $return_arr;

  }

  public static function alignment_lonlat($lon, $lat) {
    // 経度の範囲チェック
    if (abs($lon) > 180.0) {
      throw new \Exception('longitude is out of range!');
    }
    // 緯度の範囲チェック
    // 小数点11桁以下は切り捨てる
    $p = (double)pow(10, 10);
    $lat_new = ($lat >= 0) ? (floor($lat * $p) / $p) : (ceil($lat * $p) / $p);
    if (abs($lat_new) > 85.0511287798) {
      throw new \Exception('latitude is out of range!');
    }
    return $lat_new;
}

}