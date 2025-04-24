<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class DetailObjectMaster extends Model
{
    use HasFactory;

    protected $table = 'detail_object_masters';

    protected $primaryKey = 'detail_object_id';

    public function detail_object()
    {
        return $this->hasMany(SpacialDetailObject::class, "detail_object_id", "detail_object_id");
    }
}
