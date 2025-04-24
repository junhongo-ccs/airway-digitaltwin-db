<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class SpaceObject extends Model
{
    use HasFactory;

    protected $table = 'space_objects';

    protected $primaryKey = 'space_object_cd';

    use SoftDeletes;

    protected $dates = ['deleted_at'];
}
