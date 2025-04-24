<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class WeatherForecast extends Model
{
    use HasFactory;

    protected $table = 'weather_forecast';

    protected $primaryKey = 'weather_forecast_id';

    protected $guarded = [];

    use SoftDeletes;

    protected $dates = ['deleted_at'];

    public function weather_forecast_spatial()
    {
        return $this->hasMany(WeatherForecastSpatial::class, "weather_forecast_id", "weather_forecast_id");
    }

}
