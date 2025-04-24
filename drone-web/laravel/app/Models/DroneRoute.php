<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class DroneRoute extends Model
{
    use HasFactory;

    protected $table = 'drone_route';

    protected $primaryKey = 'drone_route_info_id';

    protected $guarded = [];

}
