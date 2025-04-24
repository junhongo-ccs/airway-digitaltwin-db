<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class WeatherNow extends Model
{
    use HasFactory;

    protected $table = 'weather_now';

    protected $primaryKey = 'weather_now_id';

    protected $guarded = [];

    use SoftDeletes;

    protected $dates = ['deleted_at'];

    public function weather_now_spatial()
    {
        return $this->hasMany(WeatherNowSpatial::class, "weather_now_id", "weather_now_id");
    }

}
