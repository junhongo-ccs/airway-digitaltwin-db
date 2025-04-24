<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class GroundFeatureObject extends Model
{
    use HasFactory;

    protected $table = 'ground_feature_objects';

    protected $primaryKey = 'ground_feature_object_id';

    #protected $guarded = [];

}
