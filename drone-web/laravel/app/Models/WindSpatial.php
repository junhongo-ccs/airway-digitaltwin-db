<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class WindSpatial extends Model
{
    use HasFactory;

    protected $table = 'wind_spatial';

    protected $guarded = [];

    use SoftDeletes;

    protected $dates = ['deleted_at'];

    public function wind_master()
    {
        return $this->belongsTo(Wind::class, "wind_id", "wind_id");
    }
}
