<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class RadioWaveObject extends Model
{
    use HasFactory;

    protected $table = 'radio_wave_objects';

    protected $primaryKey = 'radio_wave_object_id';

    #protected $guarded = [];

}
