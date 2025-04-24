<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class SpacialDetailObject extends Model
{
    use HasFactory;

    protected $table = 'spacial_detail_objects';

    use SoftDeletes;

    protected $dates = ['deleted_at'];

    public function detail_master()
    {
        return $this->belongsTo(DetailObjectMaster::class, "detail_object_id", "detail_object_id");
    }
}
