<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class DroneRouteSpatial extends Model
{
    use HasFactory;

    protected $table = 'drone_route_spatial';

    protected $primaryKey = 'drone_route_spatial';

}
