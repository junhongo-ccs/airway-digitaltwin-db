<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class AreaDetailObject extends Model
{
    use HasFactory;

    protected $table = 'area_detail_objects';

    use SoftDeletes;

    protected $dates = ['deleted_at'];

    public function area_master()
    {
        return $this->belongsTo(Wind::class, "area_object_id", "area_object_id");
    }
}
