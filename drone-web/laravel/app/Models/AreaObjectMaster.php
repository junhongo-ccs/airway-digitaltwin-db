<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class AreaObjectMaster extends Model
{
    use HasFactory;

    protected $table = 'area_object_masters';

    protected $primaryKey = 'area_object_id';

    protected $guarded = [];

    use SoftDeletes;

    protected $dates = ['deleted_at'];

    public function area_spatial()
    {
        return $this->hasMany(AreaDetailObject::class, "area_object_id", "area_object_id");
    }

}
