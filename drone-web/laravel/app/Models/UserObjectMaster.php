<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class UserObjectMaster extends Model
{
    use HasFactory;

    protected $table = 'user_object_masters';

    protected $primaryKey = 'user_object_id';

    protected $guarded = [];

    use SoftDeletes;

    protected $dates = ['deleted_at'];

    public function user_object()
    {
        return $this->hasMany(SpacialUserObject::class, "user_object_id", "user_object_id");
    }

}
