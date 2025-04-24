<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class FlightProhibitedAreaObject extends Model
{
    use HasFactory;

    protected $table = 'flight_prohibited_area_objects';

    //protected $primaryKey = 'flight_prohibited_area_object_id';

    protected $guarded = [];

    use SoftDeletes;

    protected $dates = ['deleted_at'];

    /*
    public function flight_prohibited_area_object_master()
    {
        return $this->hasMany(FlightProhibitedAreaObjectMaster::class, "flight_prohibited_area_object_id", "flight_prohibited_area_object_id");
    }
    */

}
