<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class Wind extends Model
{
    use HasFactory;

    protected $table = 'wind';

    protected $primaryKey = 'wind_id';

    protected $guarded = [];

    use SoftDeletes;

    protected $dates = ['deleted_at'];

    public function wind_spatial()
    {
        return $this->hasMany(WindSpatial::class, "wind_id", "wind_id");
    }

}
