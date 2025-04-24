<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class SpacialUserObject extends Model
{
    use HasFactory;

    protected $table = 'spacial_user_objects';
    protected $guarded = [];

    use SoftDeletes;

    protected $dates = ['deleted_at'];

    public function user_master()
    {
        return $this->belongsTo(UserObjectMaster::class, "user_object_id", "user_object_id");
    }

}
